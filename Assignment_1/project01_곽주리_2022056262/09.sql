-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Chang�� Ű��� �ִ� ĳ���� �� ���� ���� ������ ĳ������ ������ ����ϼ���
SELECT max(level)
FROM (
        SELECT level
        FROM user, raisingcharacter
        where
            user.id = raisingcharacter.owner_id
    ) AS chang_character;
