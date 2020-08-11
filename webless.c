#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include <webkit2/webkit2.h>

/******************************************************************************/
/*                                   Config                                   */
/******************************************************************************/

static double defaultzoom = 1.0;
static char *startpage = "https://www.duckduckgo.com/";

/* relative to home */
static char *scriptstartfile = ".config/webless/start.js";
static char *scriptendfile = ".config/webless/end.js";
static char *stylefile = ".config/webless/style.css";

static char *defaultpattern = "https://www.duckduckgo.com/?q=%s";
static char *patterns[][2] = {
	{"d",   "https://www.duckduckgo.com/?q=%s"},
	{"dl",  "https://www.duckduckgo.com/lite/?q=%s"},
	{"di",  "https://www.duckduckgo.com/?q=%s&iax=images&ia=images"},
	{"sp",  "https://www.startpage.com/do/dsearch?query=%s"},
	{"aur", "https://aur.archlinux.org/packages/?O=0&K=%s"},
	{"gh",  "https://github.com/search?q=%s"},
	{"yt",  "https://www.youtube.com/results?search_query=%s"},
	{"wi",  "https://www.wikipedia.org/?q=%s"}
};

static WebKitWebContext *context_new(void)
{
	/* web context */
	WebKitWebContext *wc = webkit_web_context_new_ephemeral();

	static char const *const languages[] = { "en-US", NULL };
	static char const *const spelling[] = { "en-US", NULL };

	/*webkit_web_context_set_additional_plugins_directory(wc, "");*/
	webkit_web_context_set_automation_allowed(wc, FALSE);
	webkit_web_context_set_cache_model(wc, WEBKIT_CACHE_MODEL_WEB_BROWSER);
	/*webkit_web_context_set_favicon_database_directory(wc, "");*/
	webkit_web_context_set_network_proxy_settings(wc, WEBKIT_NETWORK_PROXY_MODE_DEFAULT, NULL);
	webkit_web_context_set_preferred_languages(wc, languages);
	webkit_web_context_set_process_model(wc, WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);
	webkit_web_context_set_sandbox_enabled(wc, TRUE);
	webkit_web_context_set_spell_checking_enabled(wc, TRUE);
	webkit_web_context_set_spell_checking_languages(wc, spelling);
	webkit_web_context_set_tls_errors_policy(wc, WEBKIT_TLS_ERRORS_POLICY_FAIL);
	/*webkit_web_context_set_web_extensions_directory(wc, WEBEXTDIR);*/
	/*webkit_web_context_set_web_extensions_initialization_user_data(wc, NULL);*/

	/* private browsing */
	webkit_cookie_manager_set_accept_policy(
			webkit_web_context_get_cookie_manager(wc),
			WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);
	return wc;
}

static void configure_settings(WebKitSettings *ws)
{
	webkit_settings_set_default_charset(ws, "iso-8859-1,*,utf-8");
	webkit_settings_set_hardware_acceleration_policy(ws, WEBKIT_HARDWARE_ACCELERATION_POLICY_ON_DEMAND);
	webkit_settings_set_minimum_font_size(ws, 0);
	webkit_settings_set_user_agent(ws,"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36");

	webkit_settings_set_auto_load_images(ws, TRUE);
	webkit_settings_set_enable_javascript(ws, TRUE);
	webkit_settings_set_enable_webgl(ws, FALSE);

	webkit_settings_set_enable_developer_extras(ws, TRUE);
	webkit_settings_set_enable_fullscreen(ws, TRUE);
	webkit_settings_set_enable_javascript_markup(ws, TRUE);
	webkit_settings_set_enable_media(ws, TRUE);
	webkit_settings_set_enable_mediasource(ws, TRUE);
	webkit_settings_set_enable_mock_capture_devices(ws, TRUE);
	webkit_settings_set_enable_page_cache(ws, TRUE);
	webkit_settings_set_enable_resizable_text_areas(ws, TRUE);
	webkit_settings_set_enable_smooth_scrolling(ws, TRUE);
	webkit_settings_set_enable_xss_auditor(ws, TRUE);
	webkit_settings_set_media_playback_allows_inline(ws, TRUE);
	webkit_settings_set_media_playback_requires_user_gesture(ws, TRUE);
	webkit_settings_set_minimum_font_size(ws, TRUE);

	webkit_settings_set_allow_file_access_from_file_urls(ws, FALSE);
	webkit_settings_set_allow_modal_dialogs(ws, FALSE);
	webkit_settings_set_allow_top_navigation_to_data_urls(ws, FALSE);
	webkit_settings_set_allow_universal_access_from_file_urls(ws, FALSE);
	webkit_settings_set_draw_compositing_indicators(ws, FALSE);
	webkit_settings_set_enable_accelerated_2d_canvas(ws, FALSE);
	webkit_settings_set_enable_back_forward_navigation_gestures(ws, FALSE);
	webkit_settings_set_enable_caret_browsing(ws, FALSE);
	webkit_settings_set_enable_dns_prefetching(ws, FALSE);
	webkit_settings_set_enable_encrypted_media(ws, FALSE);
	webkit_settings_set_enable_frame_flattening(ws, FALSE);
	webkit_settings_set_enable_html5_database(ws, FALSE);
	webkit_settings_set_enable_html5_local_storage(ws, FALSE);
	webkit_settings_set_enable_hyperlink_auditing(ws, FALSE);
	webkit_settings_set_enable_java(ws, FALSE);
	webkit_settings_set_enable_media_capabilities(ws, FALSE);
	webkit_settings_set_enable_media_stream(ws, FALSE);
	webkit_settings_set_enable_offline_web_application_cache(ws, FALSE);
	webkit_settings_set_enable_plugins(ws, FALSE);
	webkit_settings_set_enable_site_specific_quirks(ws, FALSE);
	webkit_settings_set_enable_spatial_navigation(ws, FALSE);
	webkit_settings_set_enable_tabs_to_links(ws, FALSE);
	webkit_settings_set_enable_webaudio(ws, FALSE);
	webkit_settings_set_enable_write_console_messages_to_stdout(ws,  FALSE);
	webkit_settings_set_javascript_can_access_clipboard(ws, FALSE);
	webkit_settings_set_javascript_can_open_windows_automatically(ws, FALSE);
	webkit_settings_set_load_icons_ignoring_image_load_setting(ws, FALSE);
	webkit_settings_set_zoom_text_only(ws, FALSE);
}

static char *prompt(const char *prompt, const char *last)
{
	char *cmd = g_strdup_printf(
			"/usr/bin/echo \"%s\" | /usr/bin/dmenu -p \"%s\"",
			last, prompt);
	FILE *f = popen(cmd, "r");
	g_free(cmd);

	static char str[1024];
	str[0] = '\0';

	fgets(str, sizeof(str), f);
	pclose(f);

	char *newline = strchr(str, '\n');
	if (newline)
		*newline = '\0';

	return g_strdup(str);
}


/******************************************************************************/

#define LENGTH(x) (sizeof(x) / sizeof*(x))

#define SIG_CONN(obj, str, cb) \
	g_signal_connect(G_OBJECT(obj), str, G_CALLBACK(cb), c)

#define EVAL_JS(c, js) \
	webkit_web_view_run_javascript( \
			WEBKIT_WEB_VIEW(c->wv), js, NULL, NULL, NULL);

typedef struct {
	GtkWidget *wnd;
	GtkWidget *vbox;
	GtkWidget *progress;
	GtkWidget *wv;
} Client;

static gsize nclients;
static WebKitWebContext *webctx;
static GHashTable *patterntable;

static char *validate_url(char *text);
static void web_view_load_pattern(Client *c, char *pattern);
static void on_client_destroy(GtkWidget *widget, gpointer d);
static WebKitWebView *on_client_create(WebKitWebView *wv, WebKitNavigationAction *nav, gpointer d);
static gboolean on_client_destroy_request(WebKitWebView *wv, gpointer d);
static gboolean on_client_key(GtkWidget *widget, GdkEvent *ev, gpointer d);
static gboolean on_client_button(GtkWidget *widget, GdkEvent *ev, gpointer d);
static gboolean on_permission_request(WebKitWebView *wv, WebKitPermissionRequest *r, Client *c);
static void on_client_load_progress(GObject *obj, GParamSpec *pspec, gpointer d);
static char *read_file(char *filename);
static Client *client_new(WebKitWebView *rv, char *uri);


static char *validate_url(char *text)
{
	if (g_str_has_prefix(text, "http:") ||
	    g_str_has_prefix(text, "https:")) {
		return g_strdup(text);
	} else {
		return g_strdup_printf("https://%s", text);
	}
}

static void web_view_load_pattern(Client *c, char *pattern)
{
	pattern = g_strstrip(pattern);

	if (!pattern[0]) {
		return;
	}

	char *space = strchr(pattern, ' ');
	char *uri;

	if (space) {
		*space = '\0';
		char *keyword = g_hash_table_lookup(patterntable, pattern);
		char *search;
		if (keyword) {
			search = g_uri_escape_string(space + 1, NULL, TRUE);
		} else {
			*space = ' ';
			search = g_uri_escape_string(pattern, NULL, TRUE);
			keyword = defaultpattern;
		}
		uri = g_strdup_printf(keyword, search);
		g_free(search);
	} else if (!strchr(pattern, '.')) {
		char *search = g_uri_escape_string(pattern, NULL, TRUE);
		uri = g_strdup_printf(defaultpattern, search);
		g_free(search);
	} else {
		uri = validate_url(pattern);
	}
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(c->wv), uri);
	g_free(uri);
}

static void on_client_destroy(GtkWidget *widget, gpointer d)
{
	Client *c = (Client *)d;
	(void)widget;
	free(c);
	if (--nclients == 0) {
		gtk_main_quit();
	}
}

static WebKitWebView *on_client_create(WebKitWebView *wv, WebKitNavigationAction *nav, gpointer d)
{
	(void)wv; (void)nav; (void)d;
	return WEBKIT_WEB_VIEW(client_new(wv, NULL)->wv);
}

static gboolean on_client_destroy_request(WebKitWebView *wv, gpointer d)
{
	Client *c = (Client *)d;
	(void)wv;
	gtk_widget_destroy(c->wnd);
	return TRUE;
}

static gboolean on_client_key(GtkWidget *widget, GdkEvent *ev, gpointer d)
{
	Client *c = (Client *)d;
	WebKitWebView *wv = WEBKIT_WEB_VIEW(c->wv);

	(void)widget;

	if (ev->type != GDK_KEY_PRESS)
		return FALSE;

	if (!(ev->key.state & GDK_MOD1_MASK)) {
		return FALSE;
	}

	switch (ev->key.keyval) {
	case GDK_KEY_q: /* quit window */
		gtk_widget_destroy(c->wnd);
		return TRUE;
	case GDK_KEY_o: /* open new tab */
		client_new(NULL, startpage);
		return TRUE;
	case GDK_KEY_r: /* reload */
		webkit_web_view_reload_bypass_cache(wv);
		return TRUE;
	case GDK_KEY_c: { /* search */
		char *str = prompt("go:", webkit_web_view_get_uri(wv));
		web_view_load_pattern(c, str);
		g_free(str);
		return TRUE;
	}
	case GDK_KEY_slash: { /* find */
		char *str = prompt("find:", "");
		puts(str);
		webkit_find_controller_search(
				webkit_web_view_get_find_controller(
					wv),
				str,
				WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
				WEBKIT_FIND_OPTIONS_WRAP_AROUND,
				G_MAXUINT);
		g_free(str);
		return TRUE;
	}
	case GDK_KEY_n: /* find next */
		webkit_find_controller_search_next(
				webkit_web_view_get_find_controller(
					wv));
		return TRUE;
	case GDK_KEY_N: /* find previos */
		webkit_find_controller_search_previous(
				webkit_web_view_get_find_controller(
					wv));
		return TRUE;
	case GDK_KEY_Escape: /* stop find */
		webkit_find_controller_search_finish(
				webkit_web_view_get_find_controller(
					wv));
		return TRUE;
	case GDK_KEY_h: /* go backward in history */
		webkit_web_view_go_back(wv);
		return TRUE;
	case GDK_KEY_l: /* go forward in history */
		webkit_web_view_go_forward(wv);
		return TRUE;
	case GDK_KEY_k: /* scroll up */
		EVAL_JS(c, "window.scrollBy(0, -64);");
		return TRUE;
	case GDK_KEY_j: /* scroll down */
		EVAL_JS(c, "window.scrollBy(0, 64);");
		return TRUE;
	case GDK_KEY_u: /* scroll page up */
		EVAL_JS(c, "window.scrollBy(0, -512);");
		return TRUE;
	case GDK_KEY_d: /* scroll page down */
		EVAL_JS(c, "window.scrollBy(0, 512);");
		return TRUE;
	case GDK_KEY_g: /* scroll to top */
		EVAL_JS(c, "window.scrollTo(0, 0);");
		return TRUE;
	case GDK_KEY_G: /* scroll to bottom */
		EVAL_JS(c, "window.scrollTo(0, document.documentElement.scrollHeight);");
		return TRUE;
	case GDK_KEY_bracketleft: /* zoom out */
	case GDK_KEY_minus:
	case GDK_KEY_KP_Subtract: {
		double zoom = webkit_web_view_get_zoom_level(wv);
		zoom *=  0.9;
		webkit_web_view_set_zoom_level(wv, zoom);
		return TRUE;
	}
	case GDK_KEY_bracketright: /* zoom in */
	case GDK_KEY_plus:
	case GDK_KEY_KP_Add: {
		double zoom = webkit_web_view_get_zoom_level(wv);
		zoom *=  1.1;
		webkit_web_view_set_zoom_level(wv, zoom);
		return TRUE;
	}
	case GDK_KEY_equal: /* reset zoom */
	case GDK_KEY_KP_Equal:
		webkit_web_view_set_zoom_level(wv, defaultzoom);
		return TRUE;
	case GDK_KEY_P:
		webkit_print_operation_run_dialog(
				webkit_print_operation_new(
					wv),
				GTK_WINDOW(c->wnd));
		return TRUE;
	case GDK_KEY_I: { /* toggle web inspector */
		WebKitWebInspector *inspector =
			webkit_web_view_get_inspector(wv);
		if (webkit_web_inspector_is_attached(inspector))
			webkit_web_inspector_close(inspector);
		else
			webkit_web_inspector_show(inspector);
		return TRUE;
	}
	case GDK_KEY_J: /* toggle JavaScript */
		webkit_settings_set_enable_javascript(
				webkit_web_view_get_settings(wv),
				!webkit_settings_get_enable_javascript(
					webkit_web_view_get_settings(wv)));
		return TRUE;
	case GDK_KEY_W: /* toggle WebGL */
		webkit_settings_set_enable_webgl(
				webkit_web_view_get_settings(wv),
				!webkit_settings_get_enable_webgl(
					webkit_web_view_get_settings(wv)));
		return TRUE;
	case GDK_KEY_A: /* toggle auto load image */
		webkit_settings_set_auto_load_images(
				webkit_web_view_get_settings(wv),
				!webkit_settings_get_auto_load_images(
					webkit_web_view_get_settings(wv)));
		return TRUE;
	}
	return FALSE;
}

static gboolean on_client_button(GtkWidget *widget, GdkEvent *ev, gpointer d)
{
	Client *c = (Client *)d;

	(void)widget; (void)d;

	switch (ev->button.button) {
	case 8: /* go backward in history */
		webkit_web_view_go_back(WEBKIT_WEB_VIEW(c->wv));
		return TRUE;
	case 9: /* go forward in history */
		webkit_web_view_go_forward(WEBKIT_WEB_VIEW(c->wv));
		return TRUE;
	}
	return FALSE;
}

static void on_client_load_progress(GObject *obj, GParamSpec *pspec, gpointer d)
{
	Client *c = (Client *)d;
	double progress = webkit_web_view_get_estimated_load_progress(WEBKIT_WEB_VIEW(c->wv));
	(void)obj; (void)pspec;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(c->progress), progress);
	if (progress >= 1.0 || progress <= 0.0) {
		gtk_widget_hide(c->progress);
	} else {
		gtk_widget_show(c->progress);
	}
}

static gboolean on_permission_request(WebKitWebView *wv, WebKitPermissionRequest *r, Client *c)
{
	(void)wv;
	char *msg = NULL;
	if (WEBKIT_IS_DEVICE_INFO_PERMISSION_REQUEST(r)) {
		msg = "Allow accessing audio/video devices?";
	} else if (WEBKIT_IS_GEOLOCATION_PERMISSION_REQUEST(r)) {
		msg = "Allow accessing geolocation?";
	} else if (WEBKIT_IS_POINTER_LOCK_PERMISSION_REQUEST(r)) {
		msg = "Allow locking the pointer?";
	} else if (WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST(r)) {
		msg = "Allow accessing audio/video devices?";
	} else {
		webkit_permission_request_deny(r);
	}
	GtkWidget *dialog = gtk_message_dialog_new(
			GTK_WINDOW(c->wnd),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"%s", msg);
	gtk_widget_show(dialog);

	switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
	case GTK_RESPONSE_YES:
		webkit_permission_request_allow(r);
		break;
	default:
		webkit_permission_request_deny(r);
		break;
	}
	gtk_widget_destroy(dialog);

	return TRUE;
}


static char *read_file(char *filename)
{
	const char *homedir = g_get_home_dir();
	char *path = g_build_filename(homedir, filename, NULL);
	char *source;

	if (!g_file_get_contents(path, &source, NULL, NULL)) {
		fprintf(stderr, "webless: failed to read \"%s\"\n", path);
	}
	g_free(path);

	return source;
}

static Client *client_new(WebKitWebView *rv, char *uri)
{

	Client *c;

	c = calloc(1, sizeof(Client));

	/* setup gtk window */
	c->wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(c->wnd), "webless");
	++nclients;
	SIG_CONN(c->wnd, "destroy", on_client_destroy);

	/* setup web view */
	if (rv) {
		c->wv = webkit_web_view_new_with_related_view(rv);
	} else {
		c->wv = webkit_web_view_new_with_context(webctx);
	}
	if (uri) {
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(c->wv), uri);
	}
	webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(c->wv), defaultzoom);

	c->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	c->progress = gtk_progress_bar_new();

	configure_settings(webkit_web_view_get_settings(
				WEBKIT_WEB_VIEW(c->wv)));

	SIG_CONN(c->wv, "permission-request", on_permission_request);
	SIG_CONN(c->wv, "notify::estimated-load-progress", on_client_load_progress);
	SIG_CONN(c->wv, "create", on_client_create);
	SIG_CONN(c->wv, "close", on_client_destroy_request);
	SIG_CONN(c->wv, "key-press-event", on_client_key);
	SIG_CONN(c->wv, "button-release-event", on_client_button);

	/* load javascript and css */
	WebKitUserContentManager *ucm =
		webkit_web_view_get_user_content_manager(
				WEBKIT_WEB_VIEW(c->wv));

	char *source;

	if ((source = read_file(scriptstartfile))) {
		WebKitUserScript *script = webkit_user_script_new(
				source,
				WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
				WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
				NULL, NULL);
		g_free(source);
		webkit_user_content_manager_add_script(ucm, script);
		webkit_user_script_unref(script);
	}
	if ((source = read_file(scriptendfile))) {
		WebKitUserScript *script = webkit_user_script_new(
				source,
				WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
				WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END,
				NULL, NULL);
		g_free(source);
		webkit_user_content_manager_add_script(ucm, script);
		webkit_user_script_unref(script);
	}
	if ((source = read_file(stylefile))) {
		WebKitUserStyleSheet *style = webkit_user_style_sheet_new(
				source,
				WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
				WEBKIT_USER_STYLE_LEVEL_USER, NULL, NULL);
		webkit_user_content_manager_add_style_sheet(ucm, style);
		webkit_user_style_sheet_unref(style);
		g_free(source);
	}

	/* finish ui */
	gtk_box_pack_start(GTK_BOX(c->vbox), c->progress, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(c->vbox), c->wv, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(c->wnd), c->vbox);
	gtk_widget_grab_focus(c->wv);

	gtk_widget_show(c->wnd);
	gtk_widget_show(c->vbox);
	gtk_widget_show(c->wv);
	gtk_widget_grab_focus(c->wv);

	return c;
}

static void init(void)
{
	webctx = context_new();

	/* patterns */
	patterntable = g_hash_table_new(g_str_hash, g_str_equal);
	for (size_t i = 0; i < LENGTH(patterns); ++i) {
		g_hash_table_insert(patterntable, patterns[i][0], patterns[i][1]);
	}
}

int main(int argc, char **argv)
{
	Client *c;

	gtk_init(&argc, &argv);
	init();

	c = client_new(NULL, startpage);
	(void)c;

	gtk_main();
	exit(EXIT_SUCCESS);
}
