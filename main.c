#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <argp.h>

/* 
 *	START ARGP CRUFT
 */ 

#define MAND_ARGS 1

typedef struct args {
	char* token;
	char* url;
	char* file;
	char* string;
	char* title;
	char* body;
} args_t;

static int parse_opt(int key, char* arg, struct argp_state* state) {
	args_t* args = state->input;

	switch (key) {
		case 't':
			args->token = arg;
			break;
		case 'u':
			args->url = arg;
			break;
		case 'f':
			args->file = arg;
			break;
		case 's':
			args->string = arg;
			break;
		case 'h':
			args->title = arg;
			break;
		case 'b':
			args->body = arg;
			break;
		case ARGP_KEY_ARG:
			state->arg_num--;
			if (state->arg_num >= 0) {
				argp_usage(state);
			}
			break;
		case ARGP_KEY_END:
			if (state->arg_num >= MAND_ARGS) {
				argp_usage(state);
			}

			if (
				!args->token || 
				!args->url || 
				!args->file || 
				!args->string || 
				!args->title ||
				!args->body
			) {
				argp_usage(state);
			}

			/*
			if (state->arg_num < 0) {
				argp_failure(state, 1, 0, "too few arguments");
			}
			*/
			break;
		default:
			return 0;
	}
	return 0;
}

struct argp_option options[] = {
  //  long option name
  //    |     short option name
  //    |       |    mand arg
  //    |       |      |    options
  //    |       |      |      |       help msg             
  //    |       |      |      |          |                         ?
  //    |       |      |      |          |                         |
  //    v       v      v      v          v                         v
	{ "token", 't',  "token", 0,             "ntfy.sh user token", 0},
	{   "url", 'u',    "url", 0, "ntfy.sh server address & topic", 0},
	{  "file", 'f',   "file", 0,           "path to watched file", 0},
	{"string", 's', "string", 0,                  "target string", 0},
	{ "title", 'h',  "title", 0,             "notification title", 0},
	{  "body", 'b',   "body", 0,              "notificaiton body", 0},
	{0}
};

struct argp argp = {
	options,
	parse_opt
};

/*
 *	END ARGP CRUFT
 */

long int wind_to_end(FILE* f) {

    fseek(f, 0, SEEK_END);
    return ftell(f);

}

char find_in_buf(char* buf, char* s) {

    if (strstr(buf, s)) {
        return 1;
    }
    return 0;

}

char* cat(char* one, char* two) {

	char* c_buf = (char*)malloc(strlen(one) + strlen(two) + 1);
	strcpy(c_buf, one);
	strcat(c_buf, two);
	return c_buf;

}

void trigger_alert(args_t* args) {

	char* auth_str = cat("Authorization: Bearer ", args->token);
	char* title_str = cat("Title: ", args->title);

    CURL* c = curl_easy_init();
    struct curl_slist* ntfy_headers = NULL;
    ntfy_headers = curl_slist_append(ntfy_headers, title_str);
    ntfy_headers = curl_slist_append(
        ntfy_headers, 
		auth_str
    );

    curl_easy_setopt(c, CURLOPT_URL, args->url);
    curl_easy_setopt(c, CURLOPT_HTTPHEADER, ntfy_headers);
    curl_easy_setopt(c, CURLOPT_POSTFIELDS, args->body);

    curl_easy_perform(c);
    curl_easy_cleanup(c);
	free(auth_str);
	free(title_str);

}

int main(int argc, char** argv) {

	// parse yarrrgs
	args_t args;
	args.token  = NULL;
	args.url    = NULL;
	args.file   = NULL;
	args.string = NULL;
	args.title  = NULL;
	args.body   = NULL;

	argp_parse(&argp, argc, argv, 0, 0, &args);	

    // Open the file
    FILE* f;
    if (!(f = fopen(args.file, "r"))) {
        fprintf(stderr, "Can't open file, my guy\n");
        return 1;
    }

    // Find end of file
    fseek(f, 0, SEEK_END);
    long int prev_pos = wind_to_end(f);
    printf("Seeking to end of file (%ld bytes)\n", prev_pos);

    for(;;) {

        // move to end of file
        long int curr_pos = wind_to_end(f);
        // if end position is different, file has had data appended
        if (curr_pos > prev_pos) {

            long int delta = curr_pos - prev_pos;
            // starting from newly appended data, copy bytes to buffer until eof
            fseek(f, -delta, SEEK_CUR);
            char* buf = (char*)malloc(delta + 1);
            fread(buf, 1, delta, f);

            // check if buffer contains target string
            if (find_in_buf(buf, args.string)) {
				trigger_alert(&args);
            }

            prev_pos = curr_pos;
            free(buf);

        }

        sleep(1);

    }

    fclose(f);
    return 0;

}
