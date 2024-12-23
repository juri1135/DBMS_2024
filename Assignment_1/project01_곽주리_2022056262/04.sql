-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--나라의 이름이 U로 시작하는 모든 나라의 이름을 사전순 출력
SELECT name FROM Country where name like 'U%' ORDER BY name;
