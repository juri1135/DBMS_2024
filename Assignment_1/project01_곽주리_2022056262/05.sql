-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--�̸��� J�� �������� �ʴ� ������ ���� ���������� ���
SELECT country
FROM User
WHERE
    name not like 'J%'
ORDER BY country;
