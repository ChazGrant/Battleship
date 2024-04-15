from random import randint
from hashlib import md5
from asgiref.sync import sync_to_async


MAX_LIMIT = 1000


class Generator:
    @staticmethod
    async def generateGameId() -> str:
        """
            Создаёт идентификатор игры

            Возврашает:
                Строку, содержащую идентификатор для созданной игры
        """
        game_id = ""
        for _ in range(8):
            game_id += await sync_to_async(str)(await sync_to_async(randint)(0, MAX_LIMIT))
        return game_id

    @staticmethod
    async def generateGameInviteId(first_user_id: int, second_user_id: int) -> str:
        """
            Создаёт идентификатор приглашения в игру

            Аргументы:
                first_user_id - Идентификатор первого пользователя
                second_user_id - Идентификатор второго пользователя
            
            Возвращает:
                Строку, содержащую идентификатор приглашения
        """
        return md5(
            (str(first_user_id) + str(second_user_id)).encode())\
        .hexdigest()
