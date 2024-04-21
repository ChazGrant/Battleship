#ifndef ACTIONS_ENUMS
#define ACTIONS_ENUMS

#include <QString>
#include <QMap>


//! @brief enum всех действий, которые исходят от клиента
enum OUTGOING_ACTIONS_NAMES {
    // FriendsUpdateSocket
    DELETE_FRIEND, SEND_FRIEND_REQUEST, PROCESS_FRIEND_REQUEST,
    // GameCreatorSocket
    SEND_GAME_INVITE, CREATE_GAME, FIND_GAME,
    // GameSocket
    MAKE_TURN, QUIT_GAME, PLACE_SHIP, CONNECT_TO_GAME, GET_WEAPONS,
    // ChatSocket
    SEND_MESSAGE,
    // Общий
    SUBSCRIBE
};

//! @brief enum всех действий, которые исходят от сервера
enum INCOMING_ACTIONS_NAMES {
    // GameCreatorSocket
    INCOMIG_GAME_INVITE, GAME_CREATED, GAME_FOUND,
    // FriendsUpdateSocket
    DELETED_BY_FRIEND, FRIEND_DELETED, NEW_FRIEND_REQUEST, FRIEND_REQUEST_PROCESSED,
    // GameSocket
    CONNECTED_TO_GAME, SHIP_PLACED, TURN_MADE, OPPONENT_MADE_TURN, GAME_FINISHED, ALL_SHIPS_PLACED,
    GAME_STARTED, AVAILABLE_WEAPONS,
    // ChatSocket
    INCOMING_CHAT_MESSAGE,
    // Общий
    SUBSCRIBED
};

//! @brief QMap действий и строк, которые приходят с сервера
const QMap<INCOMING_ACTIONS_NAMES, QString> INCOMING_ACTIONS = {
    // GameCreatorSocket
    {INCOMING_ACTIONS_NAMES::INCOMIG_GAME_INVITE, "incoming_game_invite"},
    {INCOMING_ACTIONS_NAMES::GAME_CREATED, "game_created"},
    {INCOMING_ACTIONS_NAMES::GAME_FOUND, "game_found"},
    // FriendsUpdateSocket
    {INCOMING_ACTIONS_NAMES::DELETED_BY_FRIEND, "deleted_by_friend"},
    {INCOMING_ACTIONS_NAMES::FRIEND_DELETED, "friend_deleted"},
    {INCOMING_ACTIONS_NAMES::NEW_FRIEND_REQUEST, "new_friend_request"},
    {INCOMING_ACTIONS_NAMES::FRIEND_REQUEST_PROCESSED, "friend_request_processed"},
    // GameSocket
    {INCOMING_ACTIONS_NAMES::CONNECTED_TO_GAME, "connected_to_game"},
    {INCOMING_ACTIONS_NAMES::TURN_MADE, "turn_made"},
    {INCOMING_ACTIONS_NAMES::SHIP_PLACED, "ship_placed"},
    {INCOMING_ACTIONS_NAMES::OPPONENT_MADE_TURN, "opponent_made_turn"},
    {INCOMING_ACTIONS_NAMES::GAME_FINISHED, "game_finished"},
    {INCOMING_ACTIONS_NAMES::ALL_SHIPS_PLACED, "all_ships_are_placed"},
    {INCOMING_ACTIONS_NAMES::GAME_STARTED, "game_started"},
    {INCOMING_ACTIONS_NAMES::AVAILABLE_WEAPONS, "available_weapons"},
    // ChatSocket
    {INCOMING_ACTIONS_NAMES::INCOMING_CHAT_MESSAGE, "incoming_chat_message"},
    // Общий
    {INCOMING_ACTIONS_NAMES::SUBSCRIBED, "subscribed"}
};

//! @brief QMap действий и строк, которые отправляются на сервер
const QMap<OUTGOING_ACTIONS_NAMES, QString> OUTGOING_ACTIONS = {
    // GameCreatorSocket
    {OUTGOING_ACTIONS_NAMES::SEND_GAME_INVITE, "send_game_invite"},
    {OUTGOING_ACTIONS_NAMES::CREATE_GAME, "create_game"},
    {OUTGOING_ACTIONS_NAMES::FIND_GAME, "find_game"},
    // FriendsUpdateSocket
    {OUTGOING_ACTIONS_NAMES::PROCESS_FRIEND_REQUEST, "process_friend_request"},
    {OUTGOING_ACTIONS_NAMES::DELETE_FRIEND, "delete_friend"},
    {OUTGOING_ACTIONS_NAMES::SEND_FRIEND_REQUEST, "send_friend_request"},
    // GameSocket
    {OUTGOING_ACTIONS_NAMES::MAKE_TURN, "make_turn"},
    {OUTGOING_ACTIONS_NAMES::QUIT_GAME, "disconnect_from_the_game"},
    {OUTGOING_ACTIONS_NAMES::PLACE_SHIP, "place_ship"},
    {OUTGOING_ACTIONS_NAMES::CONNECT_TO_GAME, "connect_to_game"},
    {OUTGOING_ACTIONS_NAMES::GET_WEAPONS, "get_weapons"},
    // ChatSocket
    {OUTGOING_ACTIONS_NAMES::SEND_MESSAGE, "send_message"},
    // Общий
    {OUTGOING_ACTIONS_NAMES::SUBSCRIBE, "subscribe"}
};
#endif // ACTIONS_ENUMS
