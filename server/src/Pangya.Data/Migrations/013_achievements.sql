-- Conquistas (cliente JP 983; docs/protocolo/SPEC-jp983-conquistas.md). Contador por (conquista, slot): slot 0 = o da
-- etapa base (acumulado por todos os níveis), i + 1 = o da etapa i quando a conquista não tem base. Etapa fechada guarda
-- o valor do contador no fechamento. Pontos e Memorial LV são calculados das etapas fechadas (não são gravados).
create table achievement_counters (
    account_id      bigint not null references accounts(id) on delete cascade,
    achievement_tid integer not null,
    slot            smallint not null,
    value           bigint not null,
    primary key (account_id, achievement_tid, slot)
);
create table achievement_quests (
    account_id      bigint not null references accounts(id) on delete cascade,
    quest_tid       integer not null,
    achievement_tid integer not null,
    value           bigint not null,
    cleared_at      timestamptz not null default now(),
    primary key (account_id, quest_tid)
);
