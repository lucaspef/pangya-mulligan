-- Grand Prix concluídos (cliente JP 983; docs/protocolo/SPEC-jp983-grandprix.md): por grupo (TypeID_Link) a melhor
-- posição (1 = 1º, contando os bots). É a lista do 0x250 e o que libera os GPs com Lock_YN.
create table grand_prix_results (
    account_id    bigint not null references accounts(id) on delete cascade,
    link          integer not null,
    best_position integer not null,
    primary key (account_id, link)
);
