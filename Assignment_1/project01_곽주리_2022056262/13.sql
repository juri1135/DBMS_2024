-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Warrior branch의 캐릭터를 가장 많이 키우는 중인 유저의 id와 그 유저의 Warrior branch 캐릭터의 수 출력
--playable에서 branch가 warrior인 relation과 raisingchar를 natural join한다.
-- join on 쓰면 혹시 모를 위험 방지 가능!! (lab ppt 참고) 조건은 cid와 playable의 id 일치 여부
-- 여기서 owner_id만 뽑으면 됨.
-- count를 그냥 하면 warrior 키우는 user 수가 나와서 user_id로 grouping해주고
-- count 함수 써서 각 group의 행 개수를 알아내야 함
-- 알아내서 내림차순으로 정렬하고 첫 번째 tuple만 갖고 오면 됨
-- 첫 번째 tuple 갖고 오는 법... 검색해서 찾음;; LIMIT 사용하면 됨

--만약 db의 data가 변경된다면, LIMIT 사용시 공통 data는 갖고 오지 못 한다는 문제점 있음...
--with as 사용해서 id랑 캐릭터 수만 저장하고 거기서 owner_id_count가 max값과 동일한 data를 다 출력하기...
WITH
    count_char AS (
        SELECT owner_id, COUNT(owner_id) AS owner_id_count
        FROM raisingcharacter
            JOIN (
                SELECT id
                FROM playablecharacter
                WHERE
                    branch = 'Warrior'
            ) AS owner ON raisingcharacter.cid = owner.id
        GROUP BY
            owner_id
        ORDER BY owner_id_count DESC
    )
SELECT owner_id, owner_id_count
FROM count_char
WHERE
    owner_id_count = (
        SELECT MAX(owner_id_count)
        FROM count_char
    );
