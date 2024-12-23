-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ��Ʃ���� Ű��� ĳ���� �� ������ ���� ���� ĳ������ �г��� ���

WITH
    youtuber_level AS (
        SELECT nickname, level
        FROM
            youtuber
            JOIN raisingcharacter AS youtuber_max_level ON youtuber_id = owner_id
        ORDER BY level desc
    )
SELECT nickname
FROM youtuber_level
WHERE
    level = (
        SELECT max(level)
        FROM youtuber_level
    )
