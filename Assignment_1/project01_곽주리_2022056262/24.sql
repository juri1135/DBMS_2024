-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Korea 유튜버가 키우는 캐릭터 중 class가 explorer인 캐릭터의 닉네임 사전순 출력

SELECT nickname
FROM (
        SELECT cid, nickname
        FROM
            youtuber
            JOIN raisingcharacter AS youtuber_char ON youtuber_id = (
                SELECT youtuber_id
                FROM youtuber
                WHERE
                    country = 'Korea'
            )
            AND youtuber_id = owner_id
    ) AS youtuber_char
    JOIN playablecharacter AS explorer_char ON class = 'Explorer'
    and youtuber_char.cid = id
ORDER BY nickname;
