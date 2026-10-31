-- Últimos jogadores com quem cada um jogou (perfil do JP: 0x9C -> 0x10E, os 5 mais recentes).
create table recent_players (
    account_id bigint not null references accounts(id) on delete cascade,
    other_id   bigint not null references accounts(id) on delete cascade,
    played_at  timestamptz not null default now(),
    primary key (account_id, other_id)
);
create index recent_players_last on recent_players (account_id, played_at desc);
