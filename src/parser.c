struct parser_state {
    ll_node* current_node;
    lexer_match* current_match;
    const char* string;
};

void _parser_state_update(struct parser_state* state) {
    state->current_match = state->current_node ? state->current_node->data : NULL;
}

void _parser_state_init(struct parser_state* state, ll* tokens, const char* string) {
    state->current_node = tokens->head;
    state->string = string;
    _parser_state_update(state);
}

void _parser_state_step(struct parser_state* state) {
    if(state->current_node) {
        state->current_node = state->current_node->next;
    }
    _parser_state_update(state);
}
libab_result parse_tokens(ll* tokens, const char* string, tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    struct parser_state state;
    _parser_state_init(&state, tokens, string);

    return result;
}
