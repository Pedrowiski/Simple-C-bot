#include <orca/discord.h>
#include <string.h>

#include <stdlib.h>
#include <time.h>

#define BOT_TOKEN \
    ""

#define create_text_message(string) \
    struct discord_create_message_params parameters = { \
        .content = string                               \
    };                                                  \
                                                        \
    discord_create_message(client, message->channel_id, &parameters, NULL)

//=================================================================//

int create_random_number(int range)
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
    struct tm *time_info;

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

    printf("%s | %s#%s: %s\n",
        splited_time,
        message->author->username,
        message->author->discriminator,
        message->content);

    if (starts_with("&ping", message->content) == 0) {
        create_text_message("Pong!");
    } else if (starts_with("&ask", message->content) == 0) {
        char *answers[] = {
            "Sim",
            "Não",
            "Talvez",
            "Claro",
            "Não sei"
        };

        int random_number = create_random_number(sizeof(answers) / 8);
        create_text_message(answers[random_number]);
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
