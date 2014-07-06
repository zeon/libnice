/*
 * Copyright 2013 University of Chicago
 *  Contact: Bryce Allen
 * Copyright 2013 Collabora Ltd.
 *  Contact: Youness Alaoui
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
 * case the provisions of LGPL are applicable instead of those above. If you
 * wish to allow use of your version of this file only under the terms of the
 * LGPL and not to allow others to use your version of this file under the
 * MPL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the MPL or the LGPL.
 */

/*
 * Example using libnice to negotiate a UDP connection between two clients,
 * possibly on the same network or behind different NATs and/or stateful
 * firewalls.
 *
 * Build:
 *   gcc -o simple-example simple-example.c `pkg-config --cflags --libs nice`
 *
 * Run two clients, one controlling and one controlled:
 *   simple-example 0 $(host -4 -t A stun.stunprotocol.org | awk '{ print $4 }')
 *   simple-example 1 $(host -4 -t A stun.stunprotocol.org | awk '{ print $4 }')
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#include <agent.h>

#define CHUNK_SIZE 1024

static GMainLoop *gloop;
GIOChannel *io_stdin, *io_in_file;
static guint stream_id;

//static int sockfd = 0;
static struct stat stbuf;
//static struct tm* tm_info;
static time_t start = 0, stop = 0;
static FILE *in = NULL;
static FILE *out = NULL;
static off_t file_size;
static char* file_name;
static gint global_ragent_read = 0;

static int hasFileAsInut = FALSE;

static const gchar *candidate_type_name[] = {"host", "srflx", "prflx", "relay"};

static const gchar *state_name[] = {"disconnected", "gathering", "connecting",
"connected", "ready", "failed"};

static int print_local_data(NiceAgent *agent, guint stream_id,
	guint component_id);
static int parse_remote_data(NiceAgent *agent, guint stream_id,
	guint component_id, char *line);
static void cb_candidate_gathering_done(NiceAgent *agent, guint stream_id,
	gpointer data);
static void cb_new_selected_pair(NiceAgent *agent, guint stream_id,
	guint component_id, gchar *lfoundation,
	gchar *rfoundation, gpointer data);
static void cb_component_state_changed(NiceAgent *agent, guint stream_id,
	guint component_id, guint state,
	gpointer data);
static void cb_nice_recv(NiceAgent *agent, guint stream_id, guint component_id,
	guint len, gchar *buf, gpointer data);
static gboolean stdin_remote_info_cb (GIOChannel *source, GIOCondition cond,
	gpointer data);
static gboolean stdin_send_data_cb (GIOChannel *source, GIOCondition cond,
	gpointer data);
static gboolean fileio_send_data_cb (GIOChannel *source, GIOCondition cond, 
	gpointer data);
static void cb_writable (NiceAgent*agent, guint stream_id, guint component_id,
	gpointer user_data);
//static gboolean writable_cb (NiceAgent *agent, guint _stream_id, guint component_id, 
//	guint state, gpointer data);
static gboolean
fileio_send_data (gpointer data);
int
main(int argc, char *argv[])
{
 	NiceAgent *agent;
 	gchar *stun_addr = NULL;
 	guint stun_port = 0;
 	gboolean controlling;

#if 0
	if(!strcmp(argv[1],"1"))
	{
	  g_debug("creating socket");
	  struct sockaddr_in serv_addr; 

	  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	  {
	      printf("\n Error : Could not create socket \n");
	      return 1;
	  }

	  memset(&serv_addr, '0', sizeof(serv_addr)); 

	  serv_addr.sin_family = AF_INET;
	  serv_addr.sin_port = htons(5000); 

	  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	  {
	      printf("\n inet_pton error occured\n");
	      return 1;
	  } 

	  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	  {
	     printf("\n Error : Connect Failed \n");
	     return 1;
	  }
	}
#endif

  	// Parse arguments
	if (argc > 4 || argc < 2 || argv[1][1] != '\0') {
		fprintf(stderr, "Usage: %s 0|1 (receiver|sender) stun_addr [stun_port]\n", argv[0]);
		return EXIT_FAILURE;
	}
	//converting into int
	controlling = argv[1][0] - '0';
	if (controlling != 0 && controlling != 1) {
		fprintf(stderr, "Usage: %s 0|1 (receiver|sender) stun_addr [stun_port]\n", argv[0]);
		return EXIT_FAILURE;
	}

	/*if(!strcmp(argv[1],"0") && argc <= 3 ){
		g_debug("Please specify a file as argument when running in sender mode.");
		return EXIT_FAILURE;
	}*/

	if (argc > 2) {
		stun_addr = argv[2];
	    stun_port = 3478; //atoi(argv[3])
	    g_debug("Using stun server: [%s:%d]\n", stun_addr, stun_port);
	    if (argc > 3){
	    	hasFileAsInut = TRUE;
	    	file_name = argv[3];
	    	in = fopen(file_name, "rb");
			if(in == NULL)
				return EXIT_FAILURE;
	    	else {
	    		if (stat(file_name, &stbuf) == 0){
	    			file_size = stbuf.st_size;
	    			g_debug("File to be transferred: [%s], size:[%zu]", file_name, file_size);
	    		} else {
	    			g_debug("Unable to stat() the file size.");
	    			return EXIT_FAILURE;
	    		}
	    	}
	      //out = fopen (argv[2], "w");
	    }    
	}

	#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init();
	#endif

	gloop = g_main_loop_new(NULL, FALSE);
	io_stdin = g_io_channel_unix_new(fileno(stdin));

	 // Create the nice agent
	agent = nice_agent_new_reliable(g_main_loop_get_context (gloop), NICE_COMPATIBILITY_RFC5245);
	if (agent == NULL)
		g_error("Failed to create agent");

	// Set the STUN settings and controlling mode
	if (stun_addr) {
		g_object_set(G_OBJECT(agent), "stun-server", stun_addr, NULL);
		g_object_set(G_OBJECT(agent), "stun-server-port", stun_port, NULL);
	}
	g_object_set(G_OBJECT(agent), "controlling-mode", controlling, NULL);
	g_debug("Controlling mode: '[%d]\n", controlling);

	  // Connect to the signals
	g_debug("Connecting signals... ");
	g_signal_connect(G_OBJECT(agent), "candidate-gathering-done", G_CALLBACK(cb_candidate_gathering_done), NULL);
	g_debug("Signal 'candidate-gathering-done' connected.");
	g_signal_connect(G_OBJECT(agent), "new-selected-pair", G_CALLBACK(cb_new_selected_pair), NULL);
	g_debug("Signal 'new-selected-pair' connected.");
	g_signal_connect(G_OBJECT(agent), "component-state-changed", G_CALLBACK(cb_component_state_changed), NULL);
	g_debug("Signal 'component-state-changed' connected.");
	  //g_signal_connect(G_OBJECT(agent), "reliable-transport-writable", G_CALLBACK(writable_cb), NULL);
	  //g_debug("Signal 'reliable-transport-writable' connected.");

	// Create a new stream with one component
	stream_id = nice_agent_add_stream(agent, 1);
	if (stream_id == 0)
		g_error("Failed to add stream");
	else
		g_debug("Created a new stream with 1 component. sid:[%d]", stream_id);

	// Attach to the component to receive the data
	// Without this call, candidates cannot be gathered
	nice_agent_attach_recv(agent, stream_id, NICE_COMPONENT_TYPE_RTP, g_main_loop_get_context (gloop), cb_nice_recv, GUINT_TO_POINTER (controlling));
	g_debug("cb_nice_recv attached to the component.");

	//nice_agent_set_port_range (lagent, ls_id, 1, 2468, 2468);
	//g_debug("cb_nice_recv attached to the component.");

	// Start gathering local candidates
	if (!nice_agent_gather_candidates(agent, stream_id))
		g_error("Failed to start candidate gathering");

	g_debug("waiting for candidate-gathering-done signal...");

	// Run the mainloop. Everything else will happen asynchronously
	// when the candidates are done gathering.
	g_main_loop_run (gloop);

	g_debug("controlling-mode: [%d]", controlling);
	/*if(controlling == 0 && hasFileAsInut == TRUE){
		printf("\nStart transferring file... \n");

		fileio_send_data(agent);
	}*/

	g_main_loop_unref(gloop);
	g_object_unref(agent);
	g_io_channel_unref (io_stdin);

	return EXIT_SUCCESS;
}

/* this function will get called everytime a client attempts to connect 
gboolean
incoming_callback  (GSocketService *service, GSocketConnection *connection, GObject *source_object, gpointer user_data)
{
	g_print("Received Connection from client!\n");
	GInputStream * istream = g_io_stream_get_input_stream (G_IO_STREAM (connection));
	gchar message[1024];
  //g_input_stream_read  (istream, message, 1024, NULL, NULL);
  //g_print("Message was: \"%s\"\n", message);
  //nice_agent_send(agent, stream_id, 1, strlen(line), line);
	return FALSE;
}*/


static void cb_writable (NiceAgent*agent, guint s_id, guint component_id,
	gpointer user_data)
{
	guint *ls_id = user_data;
	g_debug("cb_writable\n");

	if (s_id == *ls_id && component_id == NICE_COMPONENT_TYPE_RTP) {
		g_debug ("Transport is now writable, stopping mainloop");
		*ls_id = 0;
	}
}

static void
cb_candidate_gathering_done(NiceAgent *agent, guint _stream_id,
	gpointer data)
{

	g_debug("SIGNAL candidate gathering done\n");

  // Candidate gathering is done. Send our local candidates on stdout
	printf("Copy this line to remote client:\n");
	printf("\n  ");
	print_local_data(agent, _stream_id, 1);
	printf("\n");

  // Listen on stdin for the remote candidate list
	printf("Enter remote data (single line, no wrapping):\n");
	g_io_add_watch(io_stdin, G_IO_IN, stdin_remote_info_cb, agent);
	printf("> ");
	fflush (stdout);
}

static gboolean
stdin_remote_info_cb (GIOChannel *source, GIOCondition cond, gpointer data)
{	
	NiceAgent *agent = data;
	gchar *line = NULL;
	int rval;
	gboolean ret = TRUE;

	g_debug("stdin_remote_info_cb");

	if (g_io_channel_read_line (source, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
    	// Parse remote candidate list and set it on the agent
		rval = parse_remote_data(agent, stream_id, 1, line);
		if (rval == EXIT_SUCCESS) {
	      	// Return FALSE so we stop listening to stdin since we parsed the
	      	// candidates correctly
			ret = FALSE;

			g_debug("waiting for state READY or FAILED signal...");
		} else {
			fprintf(stderr, "ERROR: failed to parse remote data\n");
			printf("Enter remote data (single line, no wrapping):\n");
			printf("> ");
			fflush (stdout);
		}
		g_free (line);
	}

	return ret;
}

static void
cb_component_state_changed(NiceAgent *agent, guint _stream_id, guint component_id, guint state, gpointer data)
{
	g_debug("SIGNAL: state changed %d %d %s[%d]\n", _stream_id, component_id, state_name[state], state);

	if (state == NICE_COMPONENT_STATE_READY) {
		NiceCandidate *local, *remote;

    	// Get current selected candidate pair and print IP address used
		if (nice_agent_get_selected_pair (agent, _stream_id, component_id, &local, &remote)) {
			gchar ipaddr[INET6_ADDRSTRLEN];

			nice_address_to_string(&local->addr, ipaddr);
			printf("\nNegotiation complete: ([%s]:%d,",ipaddr, nice_address_get_port(&local->addr));
			nice_address_to_string(&remote->addr, ipaddr);
			printf(" [%s]:%d)\n", ipaddr, nice_address_get_port(&remote->addr));

			//g_main_loop_quit (gloop);
		}

		printf("\nSend lines to remote (Ctrl-D to quit):\n");
		g_io_add_watch(io_stdin, G_IO_IN, stdin_send_data_cb, agent);
		printf("> ");
		fflush (stdout);
	} else if (state == NICE_COMPONENT_STATE_FAILED) {
		g_main_loop_quit (gloop);
	}
}

/*static gboolean
raw_send_data_cb (GIOChannel *source, GIOCondition cond, gpointer data)
{
	GError * error = NULL;

  // create the new socketservice 
	GSocketService * service = g_socket_service_new ();

  // connect to the port 
	g_socket_listener_add_inet_port ((GSocketListener*)service, 1500, NULL, &error);

  // don't forget to check for errors 
	if (error != NULL)
		g_error (error->message);

  // listen to the 'incoming' signal 
	g_signal_connect (service, "incoming", G_CALLBACK (incoming_callback), data);

  // start the socket service 
	g_socket_service_start (service);
}*/

static gboolean
stdin_send_data_cb (GIOChannel *source, GIOCondition cond, gpointer data)
{
	NiceAgent *agent = data;
	gchar *line = NULL;
	gboolean controlling = FALSE;

	g_debug("stdin_send_data_cb");

	/*if (g_io_channel_read_line (source, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		nice_agent_send(agent, stream_id, 1, strlen(line), line);
		g_free (line);
		printf("> ");
		fflush (stdout);
	} else {
		nice_agent_send(agent, stream_id, 1, 1, "\0");
    	// Ctrl-D was pressed.
		g_main_loop_quit (gloop);
	}*/

	/*g_object_get (G_OBJECT (agent), "controlling-mode", &controlling, NULL);
	g_debug("controlling-mode: [%d]", controlling);
	if(controlling == FALSE && hasFileAsInut == TRUE){
		printf("\nStart transferring file... \n");

		fileio_send_data(agent);
	}else{
		nice_agent_send(agent, stream_id, 1, 1, "\0");
		g_main_loop_quit (gloop);
	}*/

	return TRUE;
}

static gboolean
fileio_send_data (gpointer data)
{
	NiceAgent *agent = data;
  	//file descriptor
	gsize nbytes = 0;
	gint sent = 0;
	gchar *file_chunk = malloc(CHUNK_SIZE);
	gboolean reliable = FALSE;

	g_debug("fileio_send_data_cb");
	
	time(&start);

	memset(file_chunk,0,CHUNK_SIZE);

	while ( (nbytes = fread(file_chunk, sizeof(char), CHUNK_SIZE, in)) > 0) {
		sent = nice_agent_send (agent, stream_id, NICE_COMPONENT_TYPE_RTP, nbytes, file_chunk);
		if (sent == -1) sent = 0;

		if(file_size>sent && sent != -1) {
			g_debug("up");
			file_size-=sent;
		}
		else {
			file_size=0;
			g_debug("down");
		}

		g_debug ("file buffer read: [%zu], left agent sent: [%d] bytes, remaining/total size: [%zu/%d]", nbytes, (int)sent, file_size, (int)stbuf.st_size);

		//entering reliable mode
		if ((unsigned)sent < nbytes) {
			g_debug("sent:[%d], nbytes:[%zu], filesize:[%d]", sent, nbytes, stbuf.st_size);

			g_object_get (G_OBJECT (agent), "reliable", &reliable, NULL);
			g_debug ("Sending data returned -1 in %s mode", reliable?"reliable":"non-reliable");
			if (reliable) {
				gulong signal_handler;
				guint stream_id_copy = stream_id;
				signal_handler = g_signal_connect (G_OBJECT (agent), "reliable-transport-writable", G_CALLBACK (cb_writable), &stream_id_copy);
				g_debug ("Running mainloop until transport is writable");
				
				while (stream_id_copy == stream_id)
					g_main_context_iteration (NULL, TRUE);
				g_signal_handler_disconnect(G_OBJECT (agent), signal_handler);

				sent = nice_agent_send (agent, stream_id, NICE_COMPONENT_TYPE_RTCP, (nbytes - sent), file_chunk+sent);

				if(file_size>sent && sent != -1) 
					file_size-=sent;
				else 
					file_size=0;

				g_debug ("reliable: left agent resent: [%d] bytes, remaining/total size: [%zu/%d]", (int)sent, file_size, (int)stbuf.st_size);
			}
		}
	}
	return TRUE;
}

static void
cb_new_selected_pair(NiceAgent *agent, guint _stream_id,
	guint component_id, gchar *lfoundation,
	gchar *rfoundation, gpointer data)
{
	g_debug("SIGNAL: selected pair %s %s", lfoundation, rfoundation);
}

static void
cb_nice_recv(NiceAgent *agent, guint _stream_id, guint component_id, guint len, gchar *buf, gpointer data)
{
	if (len == 1 && buf[0] == '\0')
    	g_main_loop_quit (gloop);

    fflush(stdout);

    g_debug("cb_nice_recv: file_name:[%s]", "simple-example");

	out=fopen("simple-example", "ab");
	if(out == NULL)
		return EXIT_FAILURE;  
  	

	g_debug("cb_nice_recv, stream_id:[%d], component_id:[%d], len:[%d], data:[%d]", _stream_id, component_id, len, GPOINTER_TO_UINT (data));

	/* XXX: dear compiler, these are for you: */
	(void)agent; (void)stream_id; (void)component_id; (void)buf;

	/*
	* Lets ignore stun packets that got through
	*/
	//if (len < 8)
	//	return;
	
	/*if (strncmp ("12345678", buf, 8))
		return;*/

	//if (component_id == 2)
	//	return;

	if (GPOINTER_TO_UINT (data) == 1) {
		if(out) {
			int written = fwrite(buf, sizeof(char), len, out);
			g_debug("buf: [%s]", buf);
			g_debug("cb_nice_recv: Receiving agent received: [%d], file buffer written: [%d], remaining/total size: [%zu/%d]",len,written, (stbuf.st_size - global_ragent_read), (int)stbuf.st_size);
			fclose(out);

			global_ragent_read += len;

			if(global_ragent_read == stbuf.st_size){
				time(&stop);
				printf("Finished in about %.0f seconds. \n", difftime(stop, start));
			}
		}
		else{ 
			//g_main_loop_quit (global_mainloop);
			perror("ERROR OPENING FILE\n"); 
		}
		g_main_loop_quit (gloop);
	}

}

static NiceCandidate *
parse_candidate(char *scand, guint _stream_id)
{
	NiceCandidate *cand = NULL;
	NiceCandidateType ntype;
	gchar **tokens = NULL;
	guint i;

	tokens = g_strsplit (scand, ",", 5);
	for (i = 0; tokens[i]; i++);
		if (i != 5)
			goto end;

		for (i = 0; i < G_N_ELEMENTS (candidate_type_name); i++) {
			if (strcmp(tokens[4], candidate_type_name[i]) == 0) {
				ntype = i;
				break;
			}
		}
		if (i == G_N_ELEMENTS (candidate_type_name))
			goto end;

		cand = nice_candidate_new(ntype);
		cand->component_id = 1;
		cand->stream_id = _stream_id;
		cand->transport = NICE_CANDIDATE_TRANSPORT_UDP;
		strncpy(cand->foundation, tokens[0], NICE_CANDIDATE_MAX_FOUNDATION);
		cand->foundation[NICE_CANDIDATE_MAX_FOUNDATION - 1] = 0;
		cand->priority = atoi (tokens[1]);

		if (!nice_address_set_from_string(&cand->addr, tokens[2])) {
			g_message("failed to parse addr: %s", tokens[2]);
			nice_candidate_free(cand);
			cand = NULL;
			goto end;
		}

		nice_address_set_port(&cand->addr, atoi (tokens[3]));

		end:
		g_strfreev(tokens);

		return cand;
}


static int
print_local_data (NiceAgent *agent, guint _stream_id, guint component_id)
{
	int result = EXIT_FAILURE;
	gchar *local_ufrag = NULL;
	gchar *local_password = NULL;
	gchar ipaddr[INET6_ADDRSTRLEN];
	GSList *cands = NULL, *item;

	if (!nice_agent_get_local_credentials(agent, _stream_id,
		&local_ufrag, &local_password))
		goto end;

	cands = nice_agent_get_local_candidates(agent, _stream_id, component_id);
	if (cands == NULL)
		goto end;

	printf("%s %s", local_ufrag, local_password);

	for (item = cands; item; item = item->next) {
		NiceCandidate *c = (NiceCandidate *)item->data;

		nice_address_to_string(&c->addr, ipaddr);

	// (foundation),(prio),(addr),(port),(type)
		printf(" %s,%u,%s,%u,%s",
			c->foundation,
			c->priority,
			ipaddr,
			nice_address_get_port(&c->addr),
			candidate_type_name[c->type]);
	}
	printf("\n");
	result = EXIT_SUCCESS;

	end:
	if (local_ufrag)
		g_free(local_ufrag);
	if (local_password)
		g_free(local_password);
	if (cands)
		g_slist_free_full(cands, (GDestroyNotify)&nice_candidate_free);

	return result;
}


static int
parse_remote_data(NiceAgent *agent, guint _stream_id,
	guint component_id, char *line)
{
	GSList *remote_candidates = NULL;
	gchar **line_argv = NULL;
	const gchar *ufrag = NULL;
	const gchar *passwd = NULL;
	int result = EXIT_FAILURE;
	int i;

	line_argv = g_strsplit_set (line, " \t\n", 0);
	for (i = 0; line_argv && line_argv[i]; i++) {
		if (strlen (line_argv[i]) == 0)
			continue;

	// first two args are remote ufrag and password
		if (!ufrag) {
			ufrag = line_argv[i];
		} else if (!passwd) {
			passwd = line_argv[i];
		} else {
  // Remaining args are serialized canidates (at least one is required)
			NiceCandidate *c = parse_candidate(line_argv[i], _stream_id);

			if (c == NULL) {
				g_message("failed to parse candidate: %s", line_argv[i]);
				goto end;
			}
			remote_candidates = g_slist_prepend(remote_candidates, c);
		}
	}
	if (ufrag == NULL || passwd == NULL || remote_candidates == NULL) {
		g_message("line must have at least ufrag, password, and one candidate");
		goto end;
	}

	if (!nice_agent_set_remote_credentials(agent, _stream_id, ufrag, passwd)) {
		g_message("failed to set remote credentials");
		goto end;
	}

  // Note: this will trigger the start of negotiation.
		if (nice_agent_set_remote_candidates(agent, _stream_id, component_id,
			remote_candidates) < 1) {
			g_message("failed to set remote candidates");
		goto end;
	}

	result = EXIT_SUCCESS;

	end:
	if (line_argv != NULL)
		g_strfreev(line_argv);
	if (remote_candidates != NULL)
		g_slist_free_full(remote_candidates, (GDestroyNotify)&nice_candidate_free);

	return result;
}