-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ��Ʃ������ �̸��� �� ��Ʃ������ Ű��� �ִ� ĳ���͵��� ���� ���� ���� ���� ������������ ���

SELECT name, sum(level)
FROM (
        SELECT owner_id, level
        FROM raisingcharacter
    ) AS youtuber
    JOIN (
        SELECT id, name
        FROM
            youtuber
            JOIN User AS youtuber_info ON youtuber_id = id
    ) AS youtuber_char ON id = owner_id
GROUP BY
    name
ORDER BY sum(level) desc;
