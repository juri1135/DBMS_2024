-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ��� ������ Ű���� �ʴ� ĳ������ �̸��� ���� ������ ���
SELECT name
FROM playablecharacter
WHERE
    id not in(
        SELECT cid
        FROM raisingcharacter
    )
ORDER BY name;
