#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include "parse_args.h"

static void show_usage()
{
	printf("zhttpd [-r document_root]"
				  "[-p listen_port]"
				  "[-w worker_process_number]"
				  "[-q waiting_queue_length]"
				  "[-h help]\n");
	exit(0);
}

int parse_args(int argc, char **argv, http_setting_t *st)
{
	int c;

	/* set default value first */
	char pathtmp[PATH_MAX];
	getcwd(pathtmp, PATH_MAX); 
	snprintf(st->hs_docroot, PATH_MAX, "%s/html", dirname(pathtmp));
	st->hs_port = 12321;
	st->hs_wknum = 5;
	st->hs_wqlen = 5;

	while ((c = getopt(argc, argv, "r:p:w:h")) != -1) {
		switch (c) {
			case 'r':
				strncpy(st->hs_docroot, optarg, sizeof(st->hs_docroot));
				break;
			case 'p':
				st->hs_port = atoi(optarg);
				break;
			case 'w':
				st->hs_wknum = atoi(optarg);
				break;
			default:
				show_usage();
				break;
		}
	}

	printf("--Docroot:%s\n--Listen:%d\n--Worker:%d\n",
			st->hs_docroot,
			st->hs_port,
			st->hs_wknum);

	return 0;
}
