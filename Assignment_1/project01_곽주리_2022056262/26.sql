-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 각 출신 나라별로 유저가 키우는 캐릭터 중 가장 레벨이 높은 캐릭터의 레벨을 내림차순 출력
SELECT max(level)
FROM
    User
    JOIN raisingcharacter ON user.id = owner_id
GROUP BY
    country
ORDER BY max(level) desc;
