-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ��� ������ Ű��� ���� Cygnus Knight class ĳ������ ��� ����
SELECT avg(level)
FROM
    raisingcharacter
    JOIN (
        SELECT id
        FROM playablecharacter
        WHERE
            class = 'Cygnus_knights'
    ) as cygnus ON cid = cygnus.id;
