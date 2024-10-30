-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--이름이 Luminus인 캐릭터와 같은 branch를 가진 캐릭터의 이름을 사전순 출력
SELECT name
FROM playablecharacter
WHERE
    branch = (
        SELECT branch
        FROM playablecharacter
        WHERE
            name = 'Luminus'
    )
ORDER BY name;
