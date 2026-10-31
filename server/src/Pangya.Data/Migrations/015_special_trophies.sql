-- Troféus especiais (Match.iff 0x2D/0x2E/0x2F: eventos de GM, copas, Grand Prix), dados por GM (/giveitem com o
-- typeid do troféu) ou pelo Grand Prix. Aparecem no perfil (lista de troféus especiais); não são itens do inventário.
create table special_trophies (
    account_id bigint not null references accounts(id) on delete cascade,
    type_id    integer not null,
    quantity   integer not null,
    given_at   timestamptz not null default now(),
    primary key (account_id, type_id)
);
