-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- �� ��� ���󺰷� ������ Ű��� ĳ���� �� ���� ������ ���� ĳ������ ������ �������� ���
SELECT max(level)
FROM
    User
    JOIN raisingcharacter ON user.id = owner_id
GROUP BY
    country
ORDER BY max(level) desc;
