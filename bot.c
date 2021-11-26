#include <orca/discord.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "nxjson.h" //https://github.com/thestr4ng3r/nxjson
#include <curl/curl.h>

//=================================================================//
// defines
//=================================================================//

#define BOT_TOKEN \
    ""

#define SEND_TEXT_MESSAGE(string) \
    struct discord_create_message_params parameters = { \
        .content = string                               \
    };                                                  \
                                                        \
    discord_create_message(client, message->channel_id, &parameters, NULL)

#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])

#define LOG_ERR(string) \
    log_error(string);  \
    return

//=================================================================//
// Auxiliar functions
//=================================================================//

void debug_buffer(char *buffer)
{
    for (long int counter = 0; counter < strlen(buffer); counter++)
        printf("%d ", buffer[counter]);

    putchar('\n');
}

int generate_random_number(int range)
{
    srand(time(NULL));
    int random_number = rand() % range;
    return random_number;
}

int starts_with(const char *prefix, const char *string)
{
    return strncmp(prefix, string, strlen(prefix));
}

struct tm *get_time(void)
{
    time_t raw_time;
    struct tm *time_info = NULL;

    time(&raw_time);
    time_info = localtime(&raw_time);

    return time_info;
}

char *split_string(char *string, char *delimiter, int position)
{
    char *token = strtok(string, delimiter);

    for (int counter = 0; counter < position; counter++)
        token = strtok(NULL, delimiter);

    return token;
}

/*
* https://stackoverflow.com/questions/24442459/returning-formatted-string-from-
* c-function
*/

char *buffer_vsnprintf(char *format, ...)
{
    va_list args;
    va_start(args, format);
    char* buffer = NULL;

    long int bytes = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (bytes >= 0) {
        va_start(args, format);
        buffer = malloc(bytes+1);

        if (buffer) {
            vsnprintf(buffer, bytes+1, format, args);
            va_end(args);
        }
    }

    return buffer;
}

//=================================================================//
// CURL
//=================================================================//

struct data_struct {
    char *response;
    int  size;
};

void init_data_struct(struct data_struct *self)
{
    self->size     = 0;
    self->response = calloc(sizeof(char), self->size);
}

size_t curl_write_handler(void *buffer,
    size_t size,
    size_t nmemb,
    struct data_struct *user_data)
{
    long int real_size  = user_data->size + (size * nmemb);
    user_data->response = realloc(user_data->response, real_size + 1);
    user_data->size     = real_size;

    memcpy(user_data->response, buffer, real_size);
    user_data->response[real_size] = '\0';

    return real_size;
}

//=================================================================//
// Orca
//=================================================================//

void on_ready(struct discord *client, const struct discord_user *bot)
{
    log_info("Estou logado!\n");
}

void on_message(struct discord *client,
    const struct discord_user *bot,
    const struct discord_message *message)
{
    struct tm *time = get_time();
    char *splited_time = split_string(asctime(time), " ", 3);

    printf("%s|%s#%s: %s\n",
        splited_time,
        message->author->username,
        message->author->discriminator,
        message->content);

    if (message->author->bot)
        return;

    if (starts_with("&ping", message->content) == 0) {
        SEND_TEXT_MESSAGE(buffer_vsnprintf("_Pong_, %s! 🏓",
            message->author->username));
        free(parameters.content);
    } else if (starts_with("&ask", message->content) == 0) {
        char *answers[] = {
            "Sim",
            "Não",
            "Talvez",
            "Claro",
            "Não sei"
        };

        int random_number = generate_random_number(ARRAY_SIZE(answers));
        SEND_TEXT_MESSAGE(answers[random_number]);
    } else if (starts_with("&apod", message->content) == 0) {
        const char *brute_url = "https://api.nasa.gov/planetary/apod?api_key=";
        const char *api_key = "";
        const char *url = buffer_vsnprintf("%s%s", brute_url, api_key);

        CURL *curl = curl_easy_init();

        if (curl == NULL) {
            LOG_ERR("Ocorreu um erro ao tentar iniciar o curl!");
        }

        struct data_struct json_response;
        memset(&json_response, 0, sizeof(json_response));

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_handler);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_response);
        CURLcode response = curl_easy_perform(curl);

        if (response != CURLE_OK) {
            LOG_ERR("Falha ao requisitar o JSON");
        }

        const nx_json *json = nx_json_parse(json_response.response, 0);

        struct discord_embed embed = {
            .color = 0x2a9df4,
            .url = (char *) url,

            .image = &(struct discord_embed_image) {
                .url = (char *) nx_json_get(json, "url")->text_value
            },

            .title = (char *) nx_json_get(json, "title")->text_value,

            .description = (char *) nx_json_get(json,
                "explanation")->text_value,

            .footer = &(struct discord_embed_footer) {
                .text =  buffer_vsnprintf("Image author: %s",
                    nx_json_get(json, "copyright")->text_value)
            }

        };

        struct discord_create_message_params params = {
            .embed = &embed
        };

        discord_create_message(client, message->channel_id, &params, NULL);
        nx_json_free(json);
        free(embed.footer->text);
    }
}

int main(void)
{
    struct discord *client = discord_init(BOT_TOKEN);

    discord_set_on_ready(client, on_ready);
    discord_set_on_message_create(client, on_message);
    discord_run(client);

    return 0;
}
