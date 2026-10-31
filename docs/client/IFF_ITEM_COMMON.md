# IFF_ITEM_COMMON — cabeçalho comum dos itens do pangya.iff (JP 983)

Definição: `src/shared/classdefine.h` (com `C_ASSERT` de deslocamento e tamanho). Levantamento de 24/10/2026, que
substituiu o antigo `unk983_88[6]` e renomeou `Point` → `TikiQtyPerPoint`. A troca foi só de nomes: os 8 `.obj` que
usam esses campos saíram idênticos antes e depois (bytes e relocações de todas as seções que não são de debug,
comparados por símbolo).

Tamanho: **0xC0 (192 bytes)**. É o começo de todo registro de `Part`, `Ball`, `Item`, `ClubSet`, `Card`, `AuxPart`,
`SetItem`, `Character`, `Caddie`, `CaddieItem`, `Club`, `Mascot`, `Skin`, `HairStyle`, `Furniture`, `Course`,
`QuestItem`, `QuestStuff`, `CounterItem`… (versão 13 do iff JP). `Desc.iff`, `CutinInfomation.iff`, `GrandPrixData.iff`,
`Match.iff`, `AddonPart.iff` e `LevelUpPrizeItem.iff` têm registros do mesmo tamanho ou maiores, mas **não** usam este
cabeçalho (os bytes desta faixa são texto ou outros dados).

## Layout

| Desl. | Campo | Tipo | Significado |
|---|---|---|---|
| 0x00 | `Final` | bool (+3) | registro ativo |
| 0x04 | `TypeId` | u32 | ID do item; `TypeId >> 26` = grupo (2 peça, 4 set de taco, 5 bola, 6 item, 0x1c aux, 0x1f carta…) |
| 0x08 | `Name` | char[64] | nome (era 40 antes do 983) |
| 0x48 | `Level:7` / `IsUnderLvl:1` | bits | nível exigido; `IsUnderLvl` = "até este nível" em vez de "a partir dele" |
| 0x49 | `Icon` | char[40] (+3) | nome do ícone (ver "Icon: 40 ou 43") |
| 0x74 | `Price` | u32 | preço (10000000 = fora da loja) |
| 0x78 | `SalePrice` | u32 | preço de promoção (mostrado ao lado do `Price` riscado) |
| 0x7c | `UsedPrice` | u32 | revenda |
| 0x80 | `IsCash:1` `IsSalable:4` `InStock:3` | bits | Cookie (1) ou Pang (0); regras de venda/correio/loja pessoal; estado na loja (0/5/6 indisponível, 3 "só compra") |
| 0x81 | `New:1` `Hit:1` `Reserve:1` | bits | selos NEW / HIT |
| 0x82 | `TimeFlag` | u8 | item por tempo |
| 0x83 | `Time` | u8 | dias (1, 7, 15, 30…) |
| 0x84 | `TikiQtyPerPoint` | u32 | **Tiki Shop**: unidades que valem `TikiPoints` (quantidade entregue na loja de pontos) |
| 0x88 | `TikiPoints` | u32 | Tiki Points do item = preço na loja de pontos; 0 = fora dela |
| 0x8c | `Mileage` | u16 | mileage ("MP") ganha na quebra da alquimia; 0 = não quebra |
| 0x8e | `BonusProb` | u16 | chance do bônus de mileage, em % (100 em todo item ativo) |
| 0x90 | `BonusMin` | u16 | bônus mínimo de mileage |
| 0x92 | `BonusMax` | u16 | bônus máximo de mileage (sempre 2 × `BonusMin` nos dados) |
| 0x94 | `TikiType` | u32 | categoria/aba da quebra: 1, 2 ou 3 |
| 0x98 | `TikiPang` | u32 | custo em Pang por item na quebra (200; 0 = inativo) |
| 0x9c | `DateFlag` | u32 | 2 = `saleStart`/`saleEnd` valem só para a promoção, não para a venda |
| 0xa0 | `saleStart` | SYSTEMTIME | início do período |
| 0xb0 | `saleEnd` | SYSTEMTIME | fim do período |

## Provas por campo novo (0x84–0x9f)

Três fontes independentes, que concordam: (1) os dados (`pangya_jp.iff` versão 13, 17 mil registros; cópia no
container em `/tmp/c545/iff/pangya_jp.iff`), (2) o cliente 983 reconstruído e (3) dois servidores JP da comunidade:
SuperSS (`Server Lib/Projeto IOCP/TYPE/data_iff.h`, `struct TikiShopDados` e `DateDados`) e Pangya-Server-Community
(`PangyaAPI.IFF.JP/Models/General/IFFTikiShopData.cs` e `IFFDate.cs`, que trazem o deslocamento em decimal:
132 = 0x84 … 156 = 0x9c).

| Campo | Nome na comunidade | Cliente 983 | Dados |
|---|---|---|---|
| `TikiQtyPerPoint` | `qnt_per_tiki_pts` / `Tiki_Qnt_Pts` | `CPointShop::GetUnitCount983` (008518e0): quantidade para bola/item/carta; peça e set de taco = 1 | `quantidade × Mileage ≈ TikiPoints × 1000` em 8.017 de 8.207 itens ativos (o resto é arredondamento: 35 cometas × 30 = 1050) |
| `TikiPoints` | `tiki_pts` | preço na loja de pontos; `CanExchange983` (0084e350) recusa 0 | carta N = 1, R = 2, SR = 5, SC = 10; o SuperSS confere `tiki_pts × 1000 == milage_pts` |
| `Mileage` | `milage_pts` / `Mileage_Pts` | `CAlchemyMPBreak::GetItemMP` (00475f80); `IsMPBreakItem983` (00a19530) exige ≠ 0 | carta N 500 … SC 10000; set de taco 7000; cometa 30 |
| `BonusProb` | `bonus_prob` | não lê (sorteio é do servidor) | 100 em todo item ativo, 0 nos 11 inativos |
| `BonusMin/Max` | `bonus[0]` min / `bonus[1]` max | não lê | pares 1/5, 10/20, 50/100, 250/500, 1500/3000, 5000/10000 |
| `TikiType` | `tipo_tiki_shop` (válido: 1, 2, 3) | aba da quebra (`FrAlchemyMPBreakDlg`), `HasType3Item` (00476c70) avisa | tipo 1: 539 itens de Pang × 8 de cash; tipo 2: 1.525 de cash × 28 de Pang; tipo 3: 3.330 fora da loja (raros/gacha, todas as cartas) |
| `TikiPang` | `tiki_pang` (ativo só se > 0) | `CAlchemyMPBreak::GetTotalValue` (00476b20) soma por item e mostra no aviso antes de confirmar (string 0x18710) | 200 em 8.196 itens, 0 nos 11 inativos |
| `DateFlag` | `active_date` / `IFFDate.active` | `== 2` em `IsSellableItem`, `checkoutdlg` (00aa3afa, 00a9f8e5, 00a9fe82) e na ordenação do `shopmain` (00a85400…): fora do período o item segue à venda pelo preço cheio, sem "(판매기간 종료)" e sem o preço riscado; outro valor = as datas limitam a venda | 0 em todos os itens deste iff (vem de evento/servidor) |

Os textos de tela (abas e o aviso 0x18704/0x18710) não estão aqui: o `GameStringManager` do 983 baixa do servidor
oficial por HTTP. Os rótulos "Pang / cash / especial" das abas vêm da distribuição acima, não do texto.

## Icon: 40 ou 43

O espaço é de 43 bytes (0x49 até 0x73; `Price` alinhado em 0x74). `char Icon[40]` + 3 de enchimento e `char Icon[43]`
dão o mesmo layout e o mesmo código: nenhum código do 645 nem do 983 usa `sizeof(Icon)` ou copia tamanho fixo (o ícone
é sempre lido até o `\0`). O PDB do 645 não tem este struct (só tipos do motor e poucas classes do jogo), então o 40
do rebang é escolha do decomp. Ficamos com **40**: é a convenção dos outros textos do iff (`Data`, `HairTex`,
`ShirtsTex`, `FaceTex` do `sChar`, `Name` antes do 983) e o maior ícone real tem 31 caracteres (`Part.iff`). Os
servidores da comunidade usam 43 porque leem o arquivo cru. Para gerar iff: até 39 caracteres + `\0`.

## Como refazer a medição

Ler o `pangya_jp.iff` (zip) com Python: cabeçalho de 8 bytes (`u16` contagem, `u16` ligação, `u32` versão = 13), registros
de `(tamanho − 8) / contagem` bytes; descartar os arquivos listados no começo. Para cada registro com `TikiType` ≠ 0,
cruzar `struct.unpack_from('<IIHHHHIII', r, 0x84)` com `r[0x80] & 1` (cash) e `Price` (10000000 = fora da loja).
