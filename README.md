В данном проекте будет реализована утилита контроля целостности файлов.

Данная утилита будет представлять из себя демона, имеющего 2 части:

1. Клиентская часть (клиент, принимающий запросы от пользователя по добавлению файла в базу для контроля, проверка целостности файла, удаление из базы данных и приостановление контроля над ним и чего-нибудь еще).

2. Демон, который следит за файлами и, если они модифицируются, то проверяется, что файл не изменен, иначе отправляется письмо.


Зависимости можно найти в файле file-integrity/debian/control

Установка.

1. Скачать репозиторий и зайти в папку file-integrity.

2. ``` debuild ```

3. ``` dpkg -i ../file-integrity_99999.9_i386.deb ```  (запустится демон, который будет следить за файлами в базе данных, еcли таковые имеются)

4. Настроить Exim:
	1. ``` dpkg-reconfigure exim4-config ```

	2. Выбрать "интернет-сайт; прием и отправка почты напрямую, используя SMTP"(internet site; mail is sent and received directly using SMTP), на остальные вопросы ответить по дефолту.

5. fi-client (запустится клиент, через который можно добалять файлы для контроля над ними, удалять, обновлять и т.д., команды, делающие это, можно узнать, набрав help)




