#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <agent.h>

guint stream_id;
gchar buffer[] = "hello world!";
GSList *lcands = NULL;

int
main(int argc, char *argv[])
{
	// Create a nice agent
	NiceAgent *agent = nice_agent_new (NULL, NICE_COMPATIBILITY_RFC5245);

	// Connect the signals
	g_signal_connect (G_OBJECT (agent), "candidate-gathering-done",
	                  G_CALLBACK (cb_candidate_gathering_done), NULL);
	g_signal_connect (G_OBJECT (agent), "component-state-changed",
	                  G_CALLBACK (cb_component_state_changed), NULL);
	g_signal_connect (G_OBJECT (agent), "new-selected-pair",
	                  G_CALLBACK (cb_new_selected_pair), NULL);

	// Create a new stream with one component and start gathering candidates
	stream_id = nice_agent_add_stream (agent, 1);
	nice_agent_gather_candidates (agent, stream_id);

	// Attach to the component to receive the data
	nice_agent_attach_recv (agent, stream_id, 1, NULL,
	                       cb_nice_recv, NULL);

	// ... Wait until the signal candidate-gathering-done is fired ...
	lcands = nice_agent_get_local_candidates(agent, stream_id, 1);

	// ... Send local candidates to the peer and set the peer's remote candidates
	nice_agent_set_remote_candidates (agent, stream_id, 1, rcands);

	// ... Wait until the signal new-selected-pair is fired ...
	// Send our message!
	nice_agent_send (agent, stream_id, 1, sizeof(buffer), buffer);

	// Anything received will be received through the cb_nice_recv callback

	// Destroy the object
	g_object_unref(agent);
}
