-- Missão diária (cliente JP 983): o dia das pendentes, o último aceite e as missões (id, typeid, estado, contador).
create table daily_quests (
    account_id  bigint primary key references accounts(id) on delete cascade,
    day         date not null,
    accepted_at timestamptz,
    quests      jsonb not null default '[]'
);
