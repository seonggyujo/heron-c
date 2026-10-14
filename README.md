# heron-c

[Heron](https://github.com/seonggyujo/heron)(Tauri 앱)을 C로 다시 만든 여러 pane 터미널입니다.
순수 C(C11)로 썼고 Win32, ConPTY, Direct2D, DirectWrite만 씁니다. 외부 라이브러리는 없고, Release exe 하나가 약 420KB입니다.

## 내려받기

[Releases](https://github.com/seonggyujo/heron-c/releases)에서 `heron-c.exe`를 받아 바로 실행합니다. 설치 파일은 없습니다.
Windows 10 1809 이상(ConPTY)이 필요하고, Windows 11에서는 창 모서리가 둥글게 나옵니다.
서명하지 않은 exe라 처음 실행할 때 SmartScreen이 경고하면 "추가 정보"에서 "실행"을 누릅니다.

## 빌드

필요한 것: MinGW-w64 gcc(UCRT), CMake, Ninja. 예를 들어 Strawberry Perl에 들어 있는 gcc 13으로 빌드합니다.

```sh
cmake -S . -B build -G Ninja                       # Debug: 콘솔 프로그램, 로그가 보임
cmake --build build
./build/heron-c-tests.exe                          # 단위 테스트

cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release                        # GUI 프로그램
```

clangd는 `build/compile_commands.json`과 `.clangd`(MinGW 대상 지정)를 읽습니다.
`.c` 파일을 추가하면 CMake를 다시 실행해서 목록을 갱신합니다.

## 실행

```sh
./build-release/heron-c.exe              # 홈 폴더에서 시작
./build-release/heron-c.exe C:\code      # 이 폴더에서 시작
```

Release 실행본은 하나만 뜹니다. 이미 떠 있으면 두 번째 실행은 폴더를 그 창에 새 pane으로 넘기고 끝납니다.

## 있는 것

- 셸: pwsh 7, Windows PowerShell, cmd, Git Bash. ▾ 메뉴로 고르면 기억하고, + 버튼으로 엽니다.
- pane 배치: 처음에는 Heron과 같은 자동 격자입니다. 헤더를 끌어 다른 pane의 가장자리에 놓으면 그쪽으로
  나뉘고, 가운데에 놓으면 두 pane이 자리를 바꿉니다. pane 사이 틈을 끌면 크기가 바뀌고(최소 80px),
  더블클릭하면 반반이 됩니다. 이렇게 손으로 바꾸면 타이틀바에 자동 격자로 돌아가는 버튼이 나옵니다.
- 움직임: 배치가 바뀌면 pane이 새 자리로 미끄러지고, 새 pane은 커지며 나타나고, 닫은 pane은 줄어들며
  사라집니다. 출력이 오면 테두리가 잠깐 빛나고 상태 점이 깜빡입니다. 설정에서 끌 수 있습니다.
- pane 헤더: 상태 점, 셸, 종료 코드, 현재 폴더, 제목, 실행 시간, 닫기.
- exit 0이면 pane이 닫히고(설정에서 끌 수 있음), 그 밖의 종료 코드는 헤더에 배지로 남습니다.
- 설정 창(톱니 버튼): 테마(초록, 검정), 언어(한국어, 영어), 터미널 글씨 크기, 애니메이션, exit 0이면 닫기,
  지난 작업 다시 열기, Claude Code 훅, 초기화(두 번 눌러 확인).
- 에이전트 사이드바(왼쪽, 접으면 48px 번호 배지만): pane에서 도는 Claude Code를 급한 순서(사용자 필요, 끝남, 작업 중,
  대기)로 보여 줍니다. 줄마다 폴더, 모델, 작업 시간, 상태(권한 요청이면 무엇인지), 커밋 안 한 줄 수(+ −), 모델 불일치
  경고, 서브에이전트. 줄을 누르면 그 pane으로, 마우스를 올리면 그 pane 테두리가 밝아집니다. 사용자가 필요하거나 끝나면
  다른 앱이 앞에 있을 때 작업 표시줄 버튼이 깜빡입니다.
- 새 에이전트: 선택한 pane 폴더에서 새 pane을 열고 `claude`를 입력합니다. ▾ 메뉴에는 최근 세션의 폴더, git 저장소면
  worktree(`claude --worktree`), 폴더 선택 창이 있습니다.
- 최근 세션: 이 PC의 Claude Code 세션을 폴더별로(접을 수 있음) 보여 주고, 누르면 `claude --resume`으로 이어 엽니다.
  오른쪽 클릭하면 휴지통으로 보낼지 묻습니다.
- Claude Code 훅: 사이드바 안내나 설정에서 켜면 settings.json에 heron-c 훅을 넣어(백업을 남기고, 사용자 훅은 그대로)
  권한 요청, 질문, 모델 확인이 보입니다. `heron-c.exe --remove-agent-hooks`로 뺍니다.
- 하단 사용량 바: heron-limits 플러그인이 있으면 5시간·주간 한도와 에이전트별 컨텍스트 사용률(70% 노랑, 90% 빨강).
- 지난 작업 다시 열기: 닫을 때의 pane(셸, 폴더, 직접 바꾼 배치)을 다음 시작에 다시 열고, Claude Code가 돌던 pane은
  세션을 이어 엽니다.
- 터미널: 16색, 256색, 트루컬러, 굵게, 기울임, 밑줄, 대체 화면, 스크롤 영역, 마우스 보고, bracketed paste, 포커스 보고.
- 링크: URL과 OSC 8 링크는 마우스를 올리면 밑줄이 생기고 Ctrl+클릭으로 브라우저에서 열립니다(http, https).
- 파일을 pane에 끌어 놓으면 경로가 입력됩니다(공백이 있으면 따옴표).
- 한글과 CJK, 이모지는 대체 글꼴로 두 칸에 그립니다. 한글 IME 조합 중인 글자는 커서 자리에 보입니다.
- 스크롤백 5000줄(휠), 드래그 선택, 더블클릭 단어 선택, 오른쪽 클릭(선택 있으면 복사, 없으면 붙여넣기).
- 직접 그린 타이틀바, Windows 11 둥근 모서리와 그림자, 모니터 DPI에 맞춘 크기.

## 단축키

| 키 | 동작 |
| --- | --- |
| Ctrl+C | 선택한 글자가 있으면 복사, 없으면 프로그램에 ^C |
| Ctrl+V | 붙여넣기 |
| Ctrl+Shift+T | 새 pane |
| Ctrl+Tab, Ctrl+Shift+Tab | 다음, 이전 pane |
| Esc | pane을 끄는 중이면 취소, 세션 삭제 질문이 떠 있으면 닫기(셸에는 가지 않음) |
| Shift+마우스 | 프로그램이 마우스를 추적해도 선택과 스크롤은 heron-c가 |

## 아직 없는 것

Heron에 있고 여기에는 아직 없는 것: 탐색기 폴더 메뉴("heron-c에서 열기")와 설치 파일, 컬러 이모지(지금은 단색),
버튼에 마우스를 올릴 때 색이 120~150ms 동안 바뀌는 효과(지금은 바로 바뀜), 경계선 hover 선과 놓을 자리
미리보기의 120ms 페이드.

구조는 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)에 있습니다.

## 라이선스

MIT. [LICENSE](LICENSE)를 보세요.
