В данном проекте будет реализована утилита контроля целостности файлов.

Данная утилита будет представлять из себя демона, который имеющего 3 части:

1. Клиентская часть (клиент принимающий запросы от пользователя по добавлению файла в базу для контроля, проверка целостности файла, удаление из базы данных и приостановление контроля над ним и чего-нибудь еще).

2. Просто демон, проверяющий с некоторой периодичностью целостность всех файлов и уведомляющий через почту, если какой либо файл потерял целостность.

3. Демон, который следит за открытием файлов и проверяющий контрольные суммы после закрытия.