-- Bingo do lobby (JP): fichas, números sorteados (bit n-1) e linhas completas (bits 0..11).
create table bingo (
    account_id bigint primary key references accounts(id) on delete cascade,
    tries      integer not null,
    aimed      integer not null,
    lines      integer not null
);
