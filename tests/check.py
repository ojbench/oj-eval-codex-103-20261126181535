"""Check supplied examples and deterministic random games against a list model."""
import pathlib
import random
import subprocess
import sys

binary = str(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else './code').resolve())


def check(data, expected):
    result = subprocess.run([binary], input=data, text=True, capture_output=True, check=True)
    assert result.stdout.split() == expected.split(), (data, expected, result.stdout)


samples = pathlib.Path('/workspace/data/103/slay-the-balatro/testcases')
for path in sorted(samples.glob('*.in')):
    check(path.read_text(), path.with_suffix('.out').read_text())
print('All 12 supplied cases passed')

for seed in range(150):
    rng = random.Random(seed)
    mode = seed % 3 + 1
    cards = [(rng.choice(['SCORE', 'POWER', 'Multiplier', 'HeadButt']), rng.randrange(20))
             for _ in range(rng.randrange(40))]
    lines = [str(mode), str(len(cards))]
    lines += [f'{kind} {value}' if kind in ('SCORE', 'POWER') else f'SKILL {kind}'
              for kind, value in cards]
    draw, hand, discard = cards[:], [], []
    score, bonus, multiplier = 0, 0, 1
    answers = []
    for _ in range(400):
        command = rng.choice(['draw', 'finish', 'play'])
        if command == 'play' and not hand:
            command = 'draw'
        if command == 'draw':
            lines.append('draw')
            for _ in range(5):
                if not draw:
                    if mode == 1:
                        draw = discard[:]
                    elif mode == 2:
                        draw = discard[::-1]
                    else:
                        draw = discard[::2] + discard[1::2]
                    discard = []
                if draw:
                    hand.append(draw.pop(0))
        elif command == 'finish':
            lines.append('finish')
            discard.extend(hand)
            hand = []
        else:
            index = rng.randrange(len(hand))
            lines.append(f'play {index + 1}')
            kind, value = card = hand.pop(index)
            if kind == 'SCORE':
                score += (value + bonus) * multiplier
                multiplier = 1
            elif kind == 'POWER':
                bonus += value
            elif kind == 'Multiplier':
                multiplier += 1
            elif discard:
                draw.insert(0, discard.pop())
            if kind != 'POWER':
                discard.append(card)
        for name, value in [('score', score), ('hand', len(hand)),
                            ('draw_pile', len(draw)), ('discard_pile', len(discard))]:
            lines.append(f'query {name}')
            answers.append(str(value))
    check('\n'.join(lines + ['exit', '']), '\n'.join(answers))
print('All 150 randomized games passed')
