-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ������ �̸��� �� ������ Ű��� ĳ���� �̸��� ���. ���� �̸��� ���� ��, ������ Ű��� ĳ���� ���� �� ����
SELECT user_char.name, x.name char_name
FROM (
        SELECT owner_id, name
        FROM
            playablecharacter
            JOIN raisingcharacter AS char_name ON cid = playablecharacter.id
        ORDER BY name
    ) AS x
    JOIN User AS user_char ON id = owner_id
ORDER BY user_char.name, x.name;
