-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 캐릭터의 class가 explorer가 아닌 캐릭터의 수 출력
SELECT count(*) FROM playablecharacter WHERE class != 'Explorer';
