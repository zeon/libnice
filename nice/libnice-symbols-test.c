void nice_address_copy_to_sockaddr(void) { }
void nice_address_dup(void) { }
void nice_address_equal(void) { }
void nice_address_free(void) { }
void nice_address_get_port(void) { }
void nice_address_init(void) { }
void nice_address_ip_version(void) { }
void nice_address_is_private(void) { }
void nice_address_is_valid(void) { }
void nice_address_new(void) { }
void nice_address_set_from_sockaddr(void) { }
void nice_address_set_from_string(void) { }
void nice_address_set_ipv4(void) { }
void nice_address_set_ipv6(void) { }
void nice_address_set_port(void) { }
void nice_address_to_string(void) { }
void nice_agent_add_local_address(void) { }
void nice_agent_add_stream(void) { }
void nice_agent_recv(void) { }
void nice_agent_recv_messages(void) { }
void nice_agent_recv_nonblocking(void) { }
void nice_agent_recv_messages_nonblocking(void) { }
void nice_agent_attach_recv(void) { }
void nice_agent_forget_relays(void) { }
void nice_agent_gather_candidates(void) { }
void nice_agent_generate_local_candidate_sdp(void) { }
void nice_agent_generate_local_sdp(void) { }
void nice_agent_generate_local_stream_sdp(void) { }
void nice_agent_get_default_local_candidate(void) { }
void nice_agent_get_io_stream(void) { }
void nice_agent_get_local_candidates(void) { }
void nice_agent_get_local_credentials(void) { }
void nice_agent_get_remote_candidates(void) { }
void nice_agent_get_selected_pair(void) { }
void nice_agent_get_selected_socket(void) { }
void nice_agent_get_stream_name(void) { }
void nice_agent_get_type(void) { }
void nice_agent_new(void) { }
void nice_agent_new_reliable(void) { }
void nice_agent_parse_remote_candidate_sdp(void) { }
void nice_agent_parse_remote_sdp(void) { }
void nice_agent_parse_remote_stream_sdp(void) { }
void nice_agent_remove_stream(void) { }
void nice_agent_restart(void) { }
void nice_agent_restart_stream(void) { }
void nice_agent_send(void) { }
void nice_agent_send_messages_nonblocking(void) { }
void nice_agent_set_port_range(void) { }
void nice_agent_set_relay_info(void) { }
void nice_agent_set_remote_candidates(void) { }
void nice_agent_set_remote_credentials(void) { }
void nice_agent_set_selected_pair(void) { }
void nice_agent_set_selected_remote_candidate(void) { }
void nice_agent_set_software(void) { }
void nice_agent_set_stream_name(void) { }
void nice_agent_set_stream_tos(void) { }
void nice_candidate_copy(void) { }
void nice_candidate_free(void) { }
void nice_candidate_new(void) { }
void nice_component_state_to_string(void) { }
void nice_debug_disable(void) { }
void nice_debug_enable(void) { }
void nice_interfaces_get_ip_for_interface(void) { }
void nice_interfaces_get_local_interfaces(void) { }
void nice_interfaces_get_local_ips(void) { }
void nice_io_stream_new(void) { }
void nice_input_stream_new(void) { }
void nice_output_stream_new(void) { }
void pseudo_tcp_set_debug_level(void) { }
void pseudo_tcp_socket_close(void) { }
void pseudo_tcp_socket_connect(void) { }
void pseudo_tcp_socket_get_error(void) { }
void pseudo_tcp_socket_get_next_clock(void) { }
void pseudo_tcp_socket_new(void) { }
void pseudo_tcp_socket_notify_clock(void) { }
void pseudo_tcp_socket_notify_mtu(void) { }
void pseudo_tcp_socket_notify_packet(void) { }
void pseudo_tcp_socket_recv(void) { }
void pseudo_tcp_socket_send(void) { }
void stun_agent_build_unknown_attributes_error(void) { }
void stun_agent_default_validater(void) { }
void stun_agent_finish_message(void) { }
void stun_agent_forget_transaction(void) { }
void stun_agent_init(void) { }
void stun_agent_init_error(void) { }
void stun_agent_init_indication(void) { }
void stun_agent_init_request(void) { }
void stun_agent_init_response(void) { }
void stun_agent_set_software(void) { }
void stun_agent_validate(void) { }
void stun_debug_disable(void) { }
void stun_debug_enable(void) { }
void stun_message_append(void) { }
void stun_message_append32(void) { }
void stun_message_append64(void) { }
void stun_message_append_addr(void) { }
void stun_message_append_bytes(void) { }
void stun_message_append_error(void) { }
void stun_message_append_flag(void) { }
void stun_message_append_string(void) { }
void stun_message_append_xor_addr(void) { }
void stun_message_append_xor_addr_full(void) { }
void stun_message_find(void) { }
void stun_message_find32(void) { }
void stun_message_find64(void) { }
void stun_message_find_addr(void) { }
void stun_message_find_error(void) { }
void stun_message_find_flag(void) { }
void stun_message_find_string(void) { }
void stun_message_find_xor_addr(void) { }
void stun_message_find_xor_addr_full(void) { }
void stun_message_get_class(void) { }
void stun_message_get_method(void) { }
void stun_message_has_attribute(void) { }
void stun_message_has_cookie(void) { }
void stun_message_id(void) { }
void stun_message_init(void) { }
void stun_message_length(void) { }
void stun_message_validate_buffer_length(void) { }
void stun_optional(void) { }
void stun_strerror(void) { }
void stun_timer_refresh(void) { }
void stun_timer_remainder(void) { }
void stun_timer_start(void) { }
void stun_timer_start_reliable(void) { }
void stun_usage_bind_create(void) { }
void stun_usage_bind_keepalive(void) { }
void stun_usage_bind_process(void) { }
void stun_usage_bind_run(void) { }
void stun_usage_ice_conncheck_create(void) { }
void stun_usage_ice_conncheck_create_reply(void) { }
void stun_usage_ice_conncheck_priority(void) { }
void stun_usage_ice_conncheck_process(void) { }
void stun_usage_ice_conncheck_use_candidate(void) { }
void stun_usage_turn_create(void) { }
void stun_usage_turn_create_refresh(void) { }
void stun_usage_turn_process(void) { }
void stun_usage_turn_refresh_process(void) { }