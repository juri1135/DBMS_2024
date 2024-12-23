-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ���� ������ ���� ������ �̸��� ���
WITH
    count_country AS (
        SELECT country, count(*) AS count
        FROM User
        GROUP BY
            country
    )
SELECT country
FROM count_country
WHERE
    count = (
        SELECT max(count)
        FROM count_country
    );
