-- Convites de guilda (cliente JP 983: o mestre/submestre convida pelo login no mensageiro; o convidado aceita no jogo).
create table guild_invites (
    id         int generated always as identity primary key,
    guild_id   int not null references guilds(id) on delete cascade,
    account_id bigint not null references accounts(id) on delete cascade,
    inviter_id bigint not null references accounts(id) on delete cascade,
    created_at timestamptz not null default now(),
    unique (guild_id, account_id)
);
create index guild_invites_account_ix on guild_invites (account_id);
