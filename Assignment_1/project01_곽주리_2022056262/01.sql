-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--Explorer class의 캐릭터 이름을 사전순으로 출력하세요.--
--Explorer class의 수는 14--
SELECT name
from playablecharacter
where
    class = 'explorer'
order by name;
