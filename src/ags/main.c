/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags/main.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_config.h>

#include <ags/thread/ags_thread_application_context.h>
#include <ags/thread/ags_thread-posix.h>
#include <ags/thread/ags_single_thread.h>
#include <ags/thread/ags_autosave_thread.h>

#include <ags/server/ags_server_application_context.h>

#include <ags/plugin/ags_ladspa_manager.h>

#include <ags/audio/ags_audio_application_context.h>
#include <ags/audio/ags_devout.h>

#include <ags/X/ags_xorg_application_context.h>
#include <ags/thread/ags_gui_thread.h>
#include <ags/X/ags_window.h>

#include <gtk/gtk.h>
#include <libintl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <mcheck.h>

#include <X11/Xlib.h>

#include <gdk/gdk.h>

#include <sys/types.h>
#include <pwd.h>

#include <sys/mman.h>

static void ags_signal_cleanup();
void ags_signal_handler(int signr);

extern void ags_thread_resume_handler(int sig);
extern void ags_thread_suspend_handler(int sig);

static sigset_t ags_wait_mask;

extern AgsApplicationContext *ags_application_context;
extern AgsThreadApplicationContext *ags_thread_application_context;
extern AgsServerApplicationContext *ags_server_application_context;
extern AgsAudioApplicationContext *ags_audio_application_context;
extern AgsXorgApplicationContext *ags_xorg_application_context;

extern AgsConfig *ags_config;

extern AgsLadspaManager *ags_ladspa_manager;

struct sigaction ags_sigact;

void
ags_signal_handler(int signr)
{
  if(signr == SIGINT){
    //TODO:JK: do backup
    
    exit(-1);
  }else{
    sigemptyset(&(ags_sigact.sa_mask));

    if(signr == AGS_ASYNC_QUEUE_SIGNAL_HIGH){
      // pthread_yield();
    }
  }
}

static void
ags_signal_cleanup()
{
  sigemptyset(&(ags_sigact.sa_mask));
}

void
ags_main_quit(AgsApplicationContext *application_context)
{
  AgsThread *gui_thread;
  AgsThread *children;

  /* find gui thread */
  children = AGS_THREAD(application_context->main_loop)->children;

  while(children != NULL){
    if(AGS_IS_GUI_THREAD(children)){
      gui_thread = children;

      break;
    }

    children = children->next;
  }

  ags_thread_stop(gui_thread);
}

int
main(int argc, char **argv)
{
  AgsThread *audio_loop;
  AgsThread *gui_thread;
  GFile *autosave_file;
    
  gchar *filename, *autosave_filename;
  gboolean single_thread = FALSE;
  guint i;

  struct passwd *pw;
  uid_t uid;

  struct sched_param param;
  struct rlimit rl;
  struct sigaction sa;

  int result;

  const rlim_t kStackSize = 64L * 1024L * 1024L;   // min stack size = 64 Mb

  //  mtrace();
  atexit(ags_signal_cleanup);

  result = getrlimit(RLIMIT_STACK, &rl);

  /* set stack size 64M */
  if(result == 0){
    if(rl.rlim_cur < kStackSize){
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);

      if(result != 0){
	//TODO:JK
      }
    }
  }

  /* Ignore interactive and job-control signals.  */
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);

  ags_sigact.sa_handler = ags_signal_handler;
  sigemptyset(&ags_sigact.sa_mask);
  ags_sigact.sa_flags = 0;
  sigaction(SIGINT, &ags_sigact, (struct sigaction *) NULL);
  sigaction(SA_RESTART, &ags_sigact, (struct sigaction *) NULL);

  /* register signal handler */
  sigfillset(&(ags_wait_mask));
  sigdelset(&(ags_wait_mask), AGS_THREAD_SUSPEND_SIG);
  sigdelset(&(ags_wait_mask), AGS_THREAD_RESUME_SIG);

  sigfillset(&(sa.sa_mask));
  sa.sa_flags = 0;

  sa.sa_handler = ags_thread_resume_handler;
  sigaction(AGS_THREAD_RESUME_SIG, &sa, NULL);

  sa.sa_handler = ags_thread_suspend_handler;
  sigaction(AGS_THREAD_SUSPEND_SIG, &sa, NULL);

  /* get user info */
  uid = getuid();
  pw = getpwuid(uid);

  /* init gsequencer */
  ags_xorg_init_context(&argc, &argv);

  /* Declare ourself as a real time task */
  param.sched_priority = AGS_RT_PRIORITY;

  if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
    perror("sched_setscheduler failed\0");
  }

  mlockall(MCL_CURRENT | MCL_FUTURE);

  /* parse command line parameter */
  filename = NULL;

  for(i = 0; i < argc; i++){
    if(!strncmp(argv[i], "--single-thread\0", 16)){
      single_thread = TRUE;
    }else if(!strncmp(argv[i], "--filename\0", 11)){
      filename = argv[i + 1];
      i++;
    }
  }

  if(filename != NULL){
    AgsFile *file;

    file = g_object_new(AGS_TYPE_FILE,
			"filename\0", filename,
			NULL);
    ags_file_open(file);
    ags_file_read(file);
    ags_file_close(file);
  }else{
    AgsThread *audio_loop;
    AgsDevout *devout;
    AgsWindow *window;
    AgsServer *server;

    struct passwd *pw;
    uid_t uid;
    gchar *wdir, *config_file;
  
    ags_config_load_from_file(ags_config,
			      config_file);

    uid = getuid();
    pw = getpwuid(uid);

    wdir = g_strdup_printf("%s/%s\0",
			   pw->pw_dir,
			   AGS_DEFAULT_DIRECTORY);

    config_file = g_strdup_printf("%s/%s\0",
				  wdir,
				  AGS_DEFAULT_CONFIG);

    g_free(wdir);
    g_free(config_file);

    /* AgsDevout */
    devout = ags_devout_new(ags_application_context);
    g_object_set(ags_audio_application_context,
		 "soundcard\0", devout,
		 NULL);
    
      /* AgsWindow */
    window = ags_window_new(ags_application_context);
    g_object_set(ags_xorg_application_context,
		 "window\0", window,
		 "soundcard\0", devout,
		 NULL);
    g_object_ref(G_OBJECT(window));

    gtk_window_set_default_size((GtkWindow *) window, 500, 500);
    gtk_paned_set_position((GtkPaned *) window->paned, 300);

    ags_connectable_connect(AGS_CONNECTABLE(window));
    gtk_widget_show_all((GtkWidget *) window);

    /* AgsServer */
    server = ags_server_new(ags_application_context);
    g_object_set(ags_server_application_context,
		 "server\0", server,
		 NULL);

    /* AgsMainLoop */
    audio_loop = (AgsThread *) ags_audio_loop_new((GObject *) devout,
						  ags_application_context);
    g_object_set(ags_application_context,
		 "main-loop\0", audio_loop,
		 NULL);

    g_object_ref(audio_loop);
    ags_connectable_connect(AGS_CONNECTABLE(audio_loop));

    /* start thread tree */
    ags_thread_start(audio_loop);

    /* complete thread pool */
    ags_thread_pool_start(ags_thread_application_context->thread_pool);
  }

  if(!single_thread){
    GList *children;

    /* find gui thread */
    children = AGS_THREAD(ags_application_context->main_loop)->children;

    while(children != NULL){
      if(AGS_IS_GUI_THREAD(children)){
	gui_thread = children;

	break;
      }

      children = children->next;
    }

    /* start main loop */
    ags_thread_start(AGS_THREAD(ags_application_context->main_loop));

    /* join gui thread */
#ifdef _USE_PTH
    pth_join(gui_thread->thread,
	     NULL);
#else
    pthread_join(gui_thread->thread,
		 NULL);
#endif
  }else{
    AgsSingleThread *single_thread;

    /* single thread */
    single_thread = ags_single_thread_new((GObject *) ags_audio_application_context->soundcard->data);

    /* add known threads to single_thread */
    ags_thread_add_child(AGS_THREAD(single_thread),
			 audio_loop);
    
    /* start thread tree */
    ags_thread_start((AgsThread *) single_thread);
  }
    
  /* free managers */
  if(ags_ladspa_manager != NULL){
    g_object_unref(ags_ladspa_manager_get_instance());
  }
  
  /* delete autosave file */  
  autosave_filename = g_strdup_printf("%s/%s/%d-%s\0",
				      pw->pw_dir,
				      AGS_DEFAULT_DIRECTORY,
				      getpid(),
				      AGS_AUTOSAVE_THREAD_DEFAULT_FILENAME);
  
  autosave_file = g_file_new_for_path(autosave_filename);

  if(g_file_query_exists(autosave_file,
			 NULL)){
    g_file_delete(autosave_file,
		  NULL,
		  NULL);
  }

  g_free(autosave_filename);
  g_object_unref(autosave_file);

  //  muntrace();

  return(0);
}
