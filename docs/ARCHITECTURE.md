# heron-c 구조

Heron을 C로 다시 만든 터미널이다. Win32, ConPTY, Direct2D, DirectWrite만 쓰고 외부 라이브러리는 없다.
한 파일은 한 가지 일만 맡고 약 150줄을 넘지 않는다. 각 파일 맨 위에 맡은 일을 적는다.

터미널 안쪽은 꾸미지 않는다. xterm.js 기본 색, Cascadia Mono 글꼴, 프로그램이 출력한 바이트만 그린다.
예외는 터미널 속 왜가리(설정에서 끔) 하나다. 에이전트가 있는 pane의 빈 칸 위에만 덧그리고, 터미널 내용은 바꾸지 않는다.
테마(초록, 검정)와 상태 표시(셸, 폴더, 제목, 종료 코드, 실행 시간)는 타이틀바와 pane 테두리, 헤더에만 둔다.
코딩 에이전트(Claude Code)의 상태는 왼쪽 사이드바, 하단 사용량 바, pane 헤더의 점 색, 그리고 왜가리의 몸짓으로 보인다.

## 의존 방향

```
main.c → ui/ → pane/ → render/ → vt/ → base/
                     → pty/            ↗
                     → input/, layout/ ↗
             → hooks/ → agent/ → claude/ → base/
             → buddy/ → vt/, agent/
             → git/ → base/
         app/(theme, strings, prefs, paths, workspace) → base/   (pane/, agent/, ui/가 쓴다)
main.c → hooks/ (--agent-hook, --remove-agent-hooks는 창 없이 이것만)
```

- `ui/`만 창(HWND)과 Windows 메시지를 안다. 아래 모듈은 `ui/`를 모른다.
- `vt/`, `input/`, `layout/`, `buddy/`는 화면 그리기와 창을 모르는 순수 계산이다. `claude/`, `agent/`, `hooks/`, `git/`도 창을 모르고, 파일과 프로세스는 `base/`를 거쳐 다룬다. 테스트(`tests/`)는 창 없이 이 모듈들과 `base/`, `app/`만으로 빌드된다.
- 느린 일(git, `claude --version`, 세션 파일 읽기, 훅 설치)은 `ui/background.c`가 다른 스레드에서 돌리고, 끝나면 UI 스레드에서 결과를 받는다.
- Direct2D와 DirectWrite 타입은 `render/` 안에만 있다. 밖에는 `canvas.h`, `ui_text.h`, `mono_font.h`, `term_paint.h`의 불투명 API만 나간다.
- `*_internal.h`, `*_parts.h`, `*_layout.h`, `canvas_d2d.h`, `mono_font_faces.h`, `fonts.h`, `glyph_fallback.h`, `term_cursor.h`, `parser_collect.h`, `split_tree_edit.h`, `strings_tables.h`, `file_drop_paths.h`, `json_reader.h`, `session_files.h`, `sidebar_hints.h`, `sidebar_recent.h`, `sidebar_hit.h`는 같은 폴더 안에서만 include한다.

## `base/` 공용 도구

| 경로 | 역할 |
| --- | --- |
| `log.c/h` | 레벨별 로그(stderr). Debug 빌드는 콘솔 프로그램이라 띄운 터미널에 보인다. `HERON_LOG`로 레벨 변경. `log_bytes`는 제어 문자를 이스케이프해서 찍는다(출력 바이트 확인용, trace) |
| `mem.c/h` | NULL을 돌려주지 않는 할당. 메모리가 없으면 로그를 남기고 끝낸다 |
| `buf.c/h` | 늘어나는 바이트 버퍼(응답, pty 출력, 붙여넣기) |
| `utf.c/h` | UTF-8과 UTF-16 변환, 코드 포인트 하나를 UTF-8로 |
| `box.h` | 정수 픽셀 사각형과 점 포함 검사 |
| `easing.c/h` | CSS 시간 곡선(cubic-bezier). Heron의 EASE_OUT(0.2, 0.8, 0.2, 1)과 ease-in |
| `json.c/h`, `json_write.c` | JSON 값 만들기, 읽기, 고치기, 쓰기. 객체는 키 순서를 지키고 숫자는 쓴 글자 그대로 다시 쓴다(사용자의 settings.json 모양 유지). 압축 또는 두 칸 들여쓰기 |
| `json_parse.c`, `json_scalars.c`, `json_reader.h` | 엄격한 JSON 읽기(RFC 8259): 배열과 객체(깊이 128까지), 숫자와 문자열(\u 이스케이프를 UTF-8로, 짝 없는 서로게이트는 오류) |
| `file.c/h` | UTF-8 경로로 파일과 폴더 다루기: 읽기(일부 구간도), 한 번에 바꿔 쓰기, 복사, 지우기, 폴더 만들기, 정보, 목록 |
| `process.c/h` | 콘솔 창 없이 프로그램을 돌려 stdout 모으기(git, `claude --version`). 호출한 스레드를 막는다 |
| `clock.c/h` | 1970년부터의 밀리초(에이전트 파일, 세션 파일과 같은 단위) |
| `env_block.c/h` | 이 프로세스 환경에서 변수 몇 개를 바꾼 UTF-16 환경 블록(셸에 HERON_PANE, HERON_AGENT_DIR) |

## `vt/` 터미널 해석 (순수 계산)

| 경로 | 역할 |
| --- | --- |
| `cell.h` | 셀(글자, 색, 속성)과 줄, 색을 32비트에 담는 방식(기본, 팔레트 번호, RGB) |
| `line.c/h` | 줄 만들기, 채우기, 크기 바꾸기. 넓은 글자의 반쪽만 지워지지 않게 한다 |
| `scrollback.c/h` | 위로 밀려난 줄의 고리 버퍼(가득 차면 가장 오래된 줄을 버림). 줄마다 변하지 않는 id |
| `palette.c/h` | xterm.js 기본 색(흰 글자, 검은 바탕, Tango 16색, 6x6x6, 회색 24단계) |
| `wcwidth.c/h` | 글자 폭: 결합 문자 0, 한글과 CJK, 이모지 2, 나머지 1 |
| `utf8_decoder.c/h` | 바이트 단위 UTF-8 해독. 두 번에 나눠 읽힌 글자도 맞게 합친다 |
| `charset.c/h` | DEC 선 그리기 문자 집합(`ESC ( 0`) |
| `parser.c/h` | VT 상태 기계: 출력, 제어 문자, ESC, CSI, OSC로 나눠 콜백을 부른다. 의미는 모른다 |
| `parser_collect.c/h` | 파서가 읽는 동안 중간 바이트, 숫자 인자(`:` 하위 인자 포함), OSC 글자를 모은다 |
| `term.h` | 공개 API: 바이트 넣기, 크기 변경, 셀과 커서와 모드 읽기, 제목과 폴더, 응답, 스크롤백 보기 |
| `term_internal.h` | `Term` 구조체와 `term_*.c` 사이 함수 |
| `term.c` | 만들기, 해제, 바이트 넣기, 읽기 함수 |
| `term_screen.c` | 커서 이동, 스크롤(전체 화면 위쪽이면 기록으로), 탭 |
| `term_edit.c` | 지우기(ED, EL, ECH), 글자와 줄 넣기, 지우기 |
| `term_print.c` | 글자 출력(자동 줄바꿈, 넓은 글자), C0 제어 문자, ESC 시퀀스 |
| `term_csi.c` | 커서와 화면을 바꾸는 CSI |
| `term_report.c` | 응답하는 CSI: DA1, DA2, 커서 위치(DSR 6), 창 크기(18t), 모드 질의(DECRQM) |
| `term_sgr.c` | SGR: 굵게, 흐리게, 기울임, 밑줄, 반전, 숨김, 취소선, 16색, 256색, 트루컬러(`;`와 `:` 형식) |
| `term_modes.c` | DEC 모드(커서 키, 자동 줄바꿈, 커서 보임, 대체 화면 47/1047/1049, 마우스 1000/1002/1003/1006, 포커스 1004, bracketed paste 2004, 동기 출력 2026), 커서 저장과 복원 |
| `term_reset.c` | DECSTR(부분 초기화), RIS(전체 초기화) |
| `term_osc.c` | OSC 0/2 제목, OSC 7과 9;9 현재 폴더(MSYS `/c/...`와 `file://`도 Windows 경로로, 알릴 때마다 프롬프트 이벤트), OSC 8 링크, OSC 10/11 색 질의 응답 |
| `term_hyperlinks.c` | OSC 8 링크: URI는 표에 두고 셀에는 16비트 번호만. 기록으로 밀려난 줄도 같은 URI를 가리킨다 |
| `term_links.c` | 셀 아래 링크 찾기: OSC 8 링크 먼저, 없으면 자동 줄바꿈으로 이어진 논리 줄(최대 16줄)에서 URL |
| `url_scan.c/h` | 줄 글자에서 http, https URL 찾기(xterm.js web-links 규칙: 끝의 마침표 같은 문장 부호는 빼고) |
| `term_view.c` | 스크롤백 보기 위치(출력이 와도 보던 줄 유지), 줄 id로 줄 찾기 |
| `term_resize.c` | 크기 변경. 다시 흘리지 않고 자르거나 채운다(ConPTY가 크기 변경 뒤 화면을 다시 그린다). 줄이 줄면 커서 위 줄을 기록으로 |
| `selection.c/h` | 마우스 선택(줄 id 기준이라 스크롤해도 같은 글자), 단어 선택, 선택한 글자를 UTF-8로(줄바꿈 CRLF, 자동 줄바꿈은 이어 붙임) |

## `input/` 입력을 바이트로 (순수 계산)

| 경로 | 역할 |
| --- | --- |
| `keys.c/h` | 화살표, Home/End, F1-F12, Insert/Delete, PageUp/Down을 xterm 시퀀스로(수정키 포함, 앱 커서 모드). Shift+Tab, Ctrl+Space. Backspace는 DEL로, Alt는 ESC 접두. 에이전트를 멈추는 글자(수정키 없는 Esc, Ctrl+C) 판정 |
| `mouse_report.c/h` | 마우스 보고: 모드별로 원하는 이벤트, SGR(1006)과 X10 형식 |
| `paste.c/h` | 붙여넣기: 줄바꿈을 CR로, bracketed paste면 표시로 감싸고 ESC를 뺀다 |

## `layout/` pane 배치 (순수 계산)

| 경로 | 역할 |
| --- | --- |
| `tree_shape.c/h` | 분할 트리를 pane 자리 번호로 바꾼 JSON(지난 작업 저장용). 되살릴 때는 모든 pane이 한 번씩, 비율이 합 1인지 확인 |
| `split_tree.c/h` | 분할 트리(Heron의 split-tree.ts): 잎은 pane id, 분할은 가로(row) 또는 세로(column)와 비율. 자동 격자를 트리로(열 = ceil(sqrt(n)), 마지막 줄은 남은 pane이 넓게), 로그용 모양 문자열 |
| `split_tree_edit.c/h` | 트리 바꾸기: 옆에 넣기(부모와 같은 방향이면 형제로), 빼기(하나 남은 분할은 접힘), 옮기기, 자리 바꾸기 |
| `split_rects.c/h` | 트리를 픽셀 상자로. 가장자리는 정수로 반올림(글자가 흐려지지 않게), pane 사이 틈은 끌 수 있는 경계선 |
| `drop_zone.c/h` | 끈 pane을 놓을 자리: 바깥 1/4은 그쪽으로 나누기, 가운데는 자리 바꾸기. 미리보기 상자 |
| `arrangement.c/h` | 자동과 수동 배치(Heron의 split-layout.ts): 자동이면 열고 닫을 때마다 격자로 다시 짓고, 끌어 놓거나 경계선을 움직이면 수동. 수동에서 새 pane은 포커스 pane의 긴 쪽 절반, 닫으면 이웃이 차지, 하나 남거나 정리하면 자동. 트리가 바뀌면 세대 번호가 올라간다. 수동 배치를 저장용 모양으로 내보내고 되살린다 |
| `divider_drag.c/h` | 경계선 끌기(양쪽 최소 80px)와 더블클릭 반반 |

## `pty/` 셸 실행

| 경로 | 역할 |
| --- | --- |
| `pty.h` | 공개 API: 시작, 출력 가져오기, 쓰기, 크기 변경, 종료, 해제. 시작할 때 물려받은 Ctrl+C 무시 상태 풀기 |
| `pty_internal.h` | `Pty` 구조체와 스레드 진입점 |
| `pty.c` | 파이프와 의사 콘솔(`CreatePseudoConsole`) 만들기, 프로세스 실행(표준 핸들은 portable-pty처럼 명시적으로 무효), 쓰기, 크기 변경 |
| `pty_threads.c` | 읽기 스레드(64KB씩 읽어 대기 버퍼에 쌓고, 비어 있다 처음 찰 때만 창에 알림. 4MB가 넘으면 멈춤), 대기 스레드(종료 기다림, 강제 종료 요청이면 콘솔 닫고 3초 뒤 강제 종료, 콘솔 닫기, 읽기 끝 기다림, 종료 코드 알림) |
| `shell.c/h` | 셸 4종(pwsh, PowerShell, cmd, Git Bash) 찾기, 이름, 설정용 id |
| `shell_launch.c` | 셸 명령줄(인자 따옴표 규칙), 환경변수(pane이 준 것을 더해), 시작 폴더. 프롬프트마다 보이지 않는 OSC 9;9로 폴더를 알리게 하는 훅(PowerShell은 `-Command`, cmd는 `PROMPT`, Git Bash는 `PROMPT_COMMAND`). 프로필 파일은 건드리지 않는다 |

## `render/` 그리기 (Direct2D, DirectWrite)

| 경로 | 역할 |
| --- | --- |
| `canvas.c/h` | 창 하나의 Direct2D 렌더 타깃. 96 DPI로 만들어 좌표는 물리 픽셀. 글자 대비 보정을 끈 렌더링 설정(흰 글자가 한 단계 굵어 보이지 않게). 장치를 잃으면 다음 프레임에 다시 만든다 |
| `canvas_d2d.h` | render 안에서만 쓰는 렌더 타깃, 브러시, 레이어, 점선 모양 |
| `canvas_effects.c` | 삼각형, 점선 둥근 테두리, 위아래로 흐려지는 가장자리(그라데이션), 변환(크기와 위치), 반투명 그리기(레이어) |
| `shapes.c` | 사각형, 둥근 사각형, 원, 선, 자르기 |
| `fonts.c/h` | DirectWrite 팩토리, 글꼴 이름으로 face 찾기, 글리프 폭 |
| `ui_text.c/h` | 타이틀바, 헤더, 사이드바 글자: 한 줄, 세로 가운데, 넘치면 말줄임. 글꼴 굵기는 CSS 숫자 |
| `ui_text_wrap.c` | 여러 줄 안내 글: 공백에서만 줄을 바꿔 한국어 낱말이 갈리지 않게(CSS keep-all) |
| `mono_font.c/h`, `mono_font_faces.h` | 터미널 글꼴(Cascadia Mono, 없으면 Consolas) 4가지(보통, 굵게, 기울임, 굵은 기울임)와 셀 크기 |
| `glyph_fallback.c/h` | 터미널 글꼴에 없는 글자(한글, CJK, 이모지, 기호)를 빌려 올 글꼴 목록과 캐시 |
| `glyph_run.c/h` | 같은 색과 face의 글자를 셀 칸에 맞춰 그린다. 있는 글자는 한 번에, 없는 글자는 대체 글꼴로 칸 가운데에(넘치면 줄여서) |
| `cell_style.c/h` | 셀 속성을 그릴 색과 face로: 굵은 기본 8색은 밝은 색으로(xterm.js 기본), 반전, 흐리게, 숨김, 선택 영역 |
| `term_paint.c/h` | 터미널 줄 그리기: 배경 덩어리, 같은 모양의 글자 덩어리, 밑줄과 취소선, 마우스 아래 링크 밑줄 |
| `term_cursor.c/h` | 커서(블록, 밑줄, 막대, 포커스가 없으면 빈 상자)와 IME 조합 중인 글자 |

## `pane/` pane 하나와 pane 목록

| 경로 | 역할 |
| --- | --- |
| `pane.c/h` | pane 하나: 셸 시작(환경변수, 첫 프롬프트에 칠 명령), 출력을 터미널에 넣고 응답을 돌려보내기, 종료 상태, 칸 수 계산과 크기 맞추기, 현재 폴더. 애니메이션 시각(옮김, 열림, 닫힘)도 들고 있다 |
| `pane_input.c/h` | 키, 글자, 붙여넣기, 복사, 포커스 보고. 입력하면 스크롤백 보기를 맨 아래로, 선택 해제 |
| `pane_mouse.c/h` | 왼쪽 드래그 선택, 더블클릭 단어 선택, 휠 스크롤백. 프로그램이 마우스를 추적하면 보고로(Shift면 Heron이 처리). 대체 화면에서 추적이 없으면 휠을 화살표 키로 |
| `pane_paint.c/h` | pane 틀(둥근 테두리, 선택된 pane의 빛, 출력 번쩍임, 파일을 끌어 올 때 밝은 테두리, 사이드바에서 가리킨 pane의 테두리), 에이전트 상태 색의 점, 검은 터미널 영역, 터미널 그리기 |
| `pane_header.c` | 헤더: 상태 점, 셸 이름, 종료 배지, 현재 폴더, 프로그램 제목, 실행 시간, 닫기. 닫아서 끝난 셸은 종료로 보이지 않는다 |
| `pane_list.c/h` | pane 목록 세 가지: 화면에 있는 것(연 순서), 닫혀서 사라지는 중인 것(자리는 유지), 자리를 내주고 셸이 끝나기를 기다리는 것. 포커스, 찾기, 배치 저장과 되살리기 |
| `pane_arrange.c` | 배치(layout/arrangement.h)를 거쳐 열기, 닫기, 자리 내주기, 옮기기, 정리, 크기 맞추기. 새 셸은 처음부터 들어갈 크기로 시작한다 |

## `app/` 앱 전체 설정

| 경로 | 역할 |
| --- | --- |
| `theme.c/h` | 앱 틀 색 두 가지(Heron의 초록 세이지, 검정). `THEME_*`는 지금 고른 테마의 값 |
| `strings.c/h`, `strings_en.c`, `strings_ko.c`, `strings_tables.h` | 화면 글자 전부(Heron i18n과 같은 키, 한국어와 영어). 고른 적 없으면 Windows 표시 언어를 따른다. 숫자가 들어가는 문장 |
| `prefs.c/h` | `%APPDATA%\heron-c\settings.ini`에 작은 설정 저장(셸, 테마, 언어, 글씨, 애니메이션, 터미널 속 왜가리, exit 0 닫기, 지난 작업 다시 열기, 사이드바 접힘, 닫은 안내, 접은 폴더, Claude Code 훅). 읽기 실패는 기본값으로. 초기화해도 훅 선택은 남긴다 |
| `strings_time.c` | 시간 글: 실행 시간("2m 14s"), 한도 초기화까지("2h 13m"), 얼마 전("5분 전", "어제") |
| `paths.c/h` | 경로 도우미: 마지막 폴더 이름, worktree 안인지, 보여 줄 이름("heron · fix-login"), 묶음 키(소문자, 끝의 \ 없이) |
| `workspace.c/h` | 지난 작업 저장 형식(pane마다 셸, 폴더, 세션 id와 배치)과 `%APPDATA%\heron-c\workspace.json` 읽고 쓰기 |

## `claude/` Claude Code의 파일과 명령

| 경로 | 역할 |
| --- | --- |
| `claude_dir.c/h` | Claude Code 설정 폴더: `CLAUDE_CONFIG_DIR` 또는 `%USERPROFILE%\.claude` |
| `claude_version.c/h` | 버전 글자 읽기와 비교(`claude --version` 출력, 대화 기록의 version) |
| `claude_commands.c/h` | pane에 칠 명령: `claude`, `claude --worktree`, `claude --resume <id>`(id는 UUID 모양만) |
| `session_summary.c/h` | 세션 파일 끝에서 마지막 폴더와 제목 읽기, 세션 id 모양 확인 |
| `session_files.c/h` | 세션 파일 찾기, 지금 실행 중인 세션(sessions\<pid>.json) |
| `sessions.c/h` | 최근 세션 목록(실행 중이거나 폴더가 없는 것은 빼고), 이어 열 수 있는지, 휴지통으로 지우기 |

## `hooks/` heron-c의 Claude Code 훅

| 경로 | 역할 |
| --- | --- |
| `hooks.c/h` | 사용자 동의(사이드바, 설정)로 settings.json에 훅 넣기, 빼기, 있는지 보기. 넣기 전에 `claude --version`이 2.1.139 이상인지 보고, 고치기 전에 백업(`.heron-c-backup`) |
| `hook_config.c/h` | settings.json의 `hooks`를 고치는 순수 계산. `heron-c.exe --agent-hook` 항목만 우리 것이고, 사용자 훅과 Heron(heron.exe) 훅은 건드리지 않는다 |
| `hook_records.c/h` | 훅 이벤트 하나가 pane 파일에 쓸 것: 상태, 모델, 서브에이전트(Heron과 같은 형식) |
| `hook_entry.c/h` | `heron-c.exe --agent-hook`: 표준 입력의 이벤트를 pane 파일에 쓰고 끝난다. 아무것도 출력하지 않고, pane 밖이면 바로 끝난다 |

## `agent/` pane 안의 코딩 에이전트

| 경로 | 역할 |
| --- | --- |
| `agent_files.c/h` | pane 파일 위치(`%LOCALAPPDATA%\heron-c\agents`, 키는 "<pid>-<pane id>")와 이름. 형식과 환경변수(HERON_PANE, HERON_AGENT_DIR)는 Heron과 같아서 heron-limits 플러그인이 그대로 쓴다 |
| `pane_watch.c/h` | pane 하나를 한 차례 살피기: 상태, 상태줄(플러그인), 모델 파일, 서브에이전트, 대화 기록의 새 답 |
| `transcript.c/h`, `answers.c/h` | 대화 기록을 끝에서부터 따라가며 새 답마다 어느 모델이 썼는지 |
| `model_check.c/h` | 답이 왔을 때 골라 둔 모델과 실제 모델이 같은지(나중에 모델을 바꿔도 앞의 답은 그대로) |
| `agent_model.c/h` | 에이전트 하나의 상태와, 신호(훅, 제목, 키, 포커스, 프롬프트)마다 어떻게 바뀌는지. 급한 순서, 상태 색, 작업 표시줄을 깜빡일지 |
| `agent_board.c/h` | 모든 pane의 에이전트와 가장 새 사용 한도. 바뀌었는지와 무엇에서 무엇으로 |
| `usage.c/h` | 플러그인 상태 파일의 5시간·주간 한도와 컨텍스트 사용률, 70%부터 노랑 90%부터 빨강 |
| `model_label.c/h` | 모델 id를 보여 줄 이름으로("claude-opus-5-5" → "Opus 5.5") |
| `session_groups.c/h` | 최근 세션을 폴더로 묶기(가장 새 것이 아래), 새 에이전트 메뉴의 폴더(선택 pane 폴더 먼저, worktree는 빼고) |

## `git/` 폴더의 git 상태

| 경로 | 역할 |
| --- | --- |
| `git.c/h` | 저장소 맨 위 폴더 찾기, HEAD 이후 바뀐 줄 수(추적 안 된 파일 포함). 콘솔 창 없이, git의 선택적 잠금 없이 |
| `numstat.c/h` | `git diff --numstat` 합계와 새 파일 줄 수(순수 계산) |

## `buddy/` 터미널 속 왜가리 (순수 계산)

에이전트가 있는 pane마다 왜가리 한 마리와, 실행 중인 서브에이전트마다 새끼 한 마리. 터미널과 에이전트 상태는 읽기만 하고, 그리기는 `ui/buddies.c`가 한다. 위치 단위는 그림 픽셀이다.

| 경로 | 역할 |
| --- | --- |
| `sprites.h`, `sprites.c`, `sprites_heron.c`, `sprites_internal.h` | 픽셀 그림: 왜가리 자세 8가지(서기, 걷기 둘, 쪼기, 날기 둘, 외다리 잠, 날개 펴기, 12×14), 새끼 3가지, 머리 위 표시(!, ?, z), 반짝이. 글자 하나가 픽셀 하나이고 색은 `ui/buddy_paint.c`가 정한다 |
| `ground.c/h` | 지금 보이는 화면에서 빈 칸 덩어리 찾기. 빈 칸은 글자가 없고 기본 배경이며 반전, 밑줄, 취소선이 없고, 커서와 그 뒤 두 칸이 아닌 칸. 가장 아래(처음 나타날 때), 가장 가까운 곳(글자가 덮었을 때), 아무 곳(돌아다닐 때) |
| `crew.c/h`, `crew_internal.h` | 왜가리 무리 하나: 에이전트 상태로 기분 정하기(작업 중, 권한 요청 !, 질문과 대기 ?, 끝남, 쉼), 나타나고 사라질 때 300ms 흐려지기, 끝나면 반짝이 셋. 프로그램이 화면을 다시 그리는 중(동기화 출력)에는 자리 판단을 미룬다 |
| `crew_moves.c` | 빈 칸 지키기: 글자가 덮으면 가장 가까운 빈 곳으로 포물선을 그리며 날아가고(350~1100ms), 빈 곳이 없으면 사라졌다가 자리가 나면 옆에서 날아 들어온다. 일하는 동안 걷기, 쪼기, 쉬기를 번갈아 하고 9~16초마다 다른 빈 곳으로 날아간다. 여유가 있으면 글자에서 두 칸 떨어져 선다 |
| `crew_chicks.c` | 새끼(최대 6): 왜가리 옆에서 깨어나 뒤로 줄지어 따라가고, 떠날 때 반짝이를 남긴다 |
| `crew_pose.c` | 지금 그릴 그림 목록(그림, 위치, 좌우 뒤집기, 투명도)과 다음 프레임까지 기다릴 시간(움직이면 33ms, 부르는 중이면 400ms 박자, 서 있거나 자면 없음) |

## `ui/` 창

| 경로 | 역할 |
| --- | --- |
| `window.c/h` | 창 클래스 등록과 만들기, 메시지를 처리 파일로 넘기는 스위치 |
| `app.c/h` | 앱 상태(DPI에 맞춘 크기와 글꼴, 사이드바와 사용량 바의 글자 모양 표, pane 목록, 새 pane 셸, 에이전트와 사이드바 상태, 마우스와 IME와 드롭다운과 끌기 상태), CSS 픽셀 크기 바꾸기, 작업 영역(사이드바 오른쪽, 사용량 바 위)에 pane 맞추기 |
| `app_panes.c` | pane 열기(셸에 pane 파일 위치를 알리는 환경변수, 첫 프롬프트에 칠 명령), 닫기, 옮기기, 정리, 포커스. 배치가 바뀌기 전 자리를 찍어 두고 바뀐 뒤 미끄러지게 한다 |
| `app_events.c/h` | 크기, DPI, 활성화(에이전트 확인, git, 최근 세션 다시 읽기), pty 출력(제목, 폴더, 프롬프트 신호, 첫 프롬프트에 명령 입력), 셸 종료(exit 0이고 설정이 켜져 있으면 닫기) |
| `app_settings.c/h` | 설정을 실행 중인 앱에 적용하고 저장: 테마, 언어, 글씨 크기, 애니메이션, 터미널 속 왜가리, 초기화 |
| `frame.c/h` | Windows 타이틀바 없는 창: 전체가 클라이언트 영역, 크기 조절 가장자리, 끌기 영역, Windows 11 둥근 모서리와 그림자, 테마에 맞춘 테두리 |
| `titlebar.c/h`, `titlebar_parts.h`, `titlebar_buttons.c` | 타이틀바: 왼쪽 "● HERON", 오른쪽 자동 격자 버튼(수동 배치일 때만), 실행 개수(실행 중이 없으면 흐리게), 톱니, 새 터미널 split 버튼(셸 이름은 고정 폭), Windows 순서의 신호등 버튼 |
| `widgets.c/h` | 공용 컨트롤 그리기: 버튼, 묶음 버튼, 스위치, 테마 견본, 드롭다운 판, 글자 간격을 둔 대문자 제목 |
| `popover.c/h` | 한 번에 하나만 열리는 드롭다운(셸 메뉴, 설정 창, 새 에이전트 폴더 메뉴)으로 마우스와 키 보내기, 바깥 클릭과 Esc로 닫기. 열고 닫을 때 120ms 동안 4px 위, 0.98 크기, 투명에서 오고 간다 |
| `shell_menu.c/h`, `shell_menu_layout.h`, `shell_menu_paint.c` | 셸 선택 드롭다운을 앱 테마로 직접 그린다. 체크 표시, 마우스 강조, 위아래 키와 Enter. 고르면 선택만 하고 기억한다 |
| `settings_panel.h`, `settings_layout.c/h`, `settings_paint.c`, `settings_input.c` | 설정 창: 테마, 언어, 글씨 크기(A−, A+), 애니메이션, 터미널 속 왜가리, exit 0이면 닫기, 지난 작업 다시 열기, Claude Code 훅(실패하면 이유를 아래에 11px로), 초기화(3초 안에 한 번 더 눌러야 실행, 끝나면 "완료"). 누른 스위치 손잡이는 120ms 동안 미끄러진다 |
| `empty_state.c/h` | pane이 없을 때 큰 + 버튼과 깜빡이는 커서가 붙은 "새 터미널_" |
| `animation.c/h` | 프레임 타이머: 출력 번쩍임, 포커스 빛(200ms로 옮겨 감), 상태 점 깜빡임, 빈 화면(마지막 pane이 닫히고 120ms 뒤 200ms로 나타남)과 커서, pane 움직임, 사이드바 폭, 드롭다운, 터미널 속 왜가리. 움직이는 것이 있을 때만 돈다 |
| `pane_motion.c/h` | pane 움직임(Heron의 flip.ts, pane-motion.ts): 새 자리로 220ms 미끄러지기, 0.96에서 커지며 220ms 나타나기, 0.94로 줄며 180ms 사라지기. 그리는 위치만 움직이고 터미널 크기는 바로 바뀐다 |
| `dividers.c/h` | pane 사이 틈: 마우스를 올리면 2px 선, 크기 조절 커서, 끌어서 크기 바꾸기, 더블클릭 반반. 끄는 중에 pane이 열리거나 닫히면 끌기를 멈춘다 |
| `header_drag.c/h` | pane 헤더 끌기: 5px 넘게 움직이면 시작, 끄는 pane은 반투명, 놓을 자리 미리보기(자리 바꾸기는 점선), Esc 취소 |
| `buddies.c/h`, `buddies_state.h` | 터미널 속 왜가리: 에이전트가 있는 pane마다 무리(`buddy/crew.h`)를 만들고, 프레임마다 움직이고, pane 그림 위에 터미널 영역으로 잘라 그린다. 그림 픽셀 크기는 왜가리와 머리 위 표시가 세 줄이 되게. 설정 > 터미널 속 왜가리와 애니메이션이 둘 다 켜져 있을 때만 |
| `buddy_paint.c/h` | 그림 글자를 테마 색 사각형으로(몸 fg_soft, 날개 fg_dim, 부리와 다리 호박색, 새끼 노랑, !와 ? 도움 필요 노랑). 가장자리를 기기 픽셀에 맞춰 이음매 없이 |
| `paint.c/h` | WM_PAINT: 타이틀바, 빈 화면 또는 pane들(그 위에 왜가리, 사라지는 pane은 아래에, 사이드바 폭이 움직이는 동안 작업 영역을 따라 늘고 줄며), 경계선, 사이드바, 사용량 바, 놓을 자리, 드롭다운 |
| `keyboard.c/h` | 끌기 취소(Esc), 드롭다운 키, 세션 삭제 질문 닫기(Esc), 단축키(선택 있으면 Ctrl+C 복사, Ctrl+V 붙여넣기, Ctrl+Shift+T 새 pane, Ctrl+Tab 다음 pane), 그다음 포커스된 pane으로. Esc나 Ctrl+C만 누르면 에이전트가 멈춘 것으로 |
| `ime.c/h` | IME: 조합 중인 글자는 앱이 커서 자리에 그리고, 완성된 글자는 pane으로. 후보 창은 커서 옆에 |
| `mouse.c/h` | 타이틀바 버튼, pane 닫기 버튼, 빈 화면 버튼의 hover와 클릭, 경계선과 헤더 끌기, 사이드바, 사용량 바 칩으로 보내기. pane 위에서는 포커스, 선택, 오른쪽 클릭(선택 있으면 복사, 없으면 붙여넣기) |
| `mouse_cursor.c` | 커서 모양: 끄는 중 이동 화살표, 경계선 위 크기 조절, 드롭다운 위 화살표, 링크 위 손, 터미널 글자 위 I자 |
| `mouse_wheel.c` | 휠을 한 칸 단위로 모아(터치패드의 작은 움직임도 더해서) 사이드바 목록이나 pane에 |
| `links.c/h` | 링크: 마우스 아래 링크에 밑줄, Ctrl+클릭하면 http, https만 기본 브라우저로 연다. 마우스를 추적하는 프로그램에 그 클릭을 넘기지 않는다 |
| `file_drop.c/h`, `file_drop_paths.c/h` | 파일 끌어 놓기(C로 쓴 IDropTarget COM 객체): 끌어 오는 동안 그 pane 테두리를 밝게, 놓으면 경로를 입력(필요하면 따옴표, 공백으로 구분) |
| `clipboard.c/h` | 클립보드 글자 읽고 쓰기(UTF-8) |
| `tooltip.c/h` | 툴팁(Heron의 title 속성): Windows 툴팁 컨트롤 하나를 마우스 아래 대상 상자로 옮겨 가며 쓴다 |
| `background.c/h` | 느린 일을 새 스레드에서 돌리고, 끝나면 WM_JOB_DONE으로 UI 스레드에서 마무리 |
| `agent_state.h` | 창이 아는 에이전트 정보(데이터만): 보드와 pane 감시, 최근 세션과 폴더 묶음, 폴더별 git 변경, 훅 켜짐 |
| `agents.c/h` | Heron의 wire-agents.ts: 0.7초마다 pane 파일을, pane 신호(제목, 프롬프트, 키, 포커스, 닫기)도 보드로. 바뀌면 다시 그리고, 사용자가 필요하거나 끝난 에이전트는 다른 앱이 앞에 있을 때 작업 표시줄을 깜빡인다 |
| `agent_rows.c/h` | 사이드바와 사용량 바가 쓰는 에이전트 행(번호, 폴더 이름, 폴더, 에이전트, git 변경), 급한 순서 정렬 |
| `changes.c/h` | 에이전트 폴더의 git 변경: 상태가 바뀔 때, 창이 포커스를 얻을 때, 일하는 동안 15초마다 백그라운드로 |
| `recent.c/h` | 최근 세션 읽기(시작, 포커스, 세션이 pane을 떠날 때), pane에서 실행 중인 세션을 빼고 폴더로 묶기, 이어 열기, 지우기 |
| `hooks_state.c/h` | 훅 켜기와 끄기, 시작할 때 확인(켜 두었는데 빠졌으면 다시 넣기), 실패 이유 문장 |
| `sidebar.c/h`, `sidebar_state.h` | 에이전트 사이드바: 기억하는 선택(접힘, 닫은 안내, 접은 폴더), 48px와 256px 사이 160ms 폭 애니메이션, 1초 시계 |
| `sidebar_layout.c/h` | 사이드바 세로 배치: 38px 머리, 36px 새 에이전트 버튼, 에이전트 행(최근 세션이 자리가 필요하면 줄고 스크롤), 빈 줄, 최근 세션(남는 자리, 최소 96px), 아래 안내 |
| `sidebar_paint.c` | 사이드바 그리기: 배경과 오른쪽 테두리, 제목과 ‹ › 버튼, 새 에이전트 split 버튼, 행, 빈 줄, 최근 세션, 안내 |
| `agent_row.c/h`, `agent_row_paint.c` | 에이전트 한 줄: 상태 색 배지, 폴더, 모델 칩, 작업 시간, 상태줄(권한 요청이면 무엇인지), +추가 −삭제 줄 수, 모델 불일치 경고, 서브에이전트 세 줄과 "외 N개", 툴팁 |
| `sidebar_recent.c/h`, `sidebar_recent_paint.c` | 최근 세션: 폴더 머리(▾/▸, 이름, 개수, 얼마 전)로 접기, 행(마우스를 올리면 "이어 열기 ↵"), 오른쪽 클릭 삭제 질문, 휠 스크롤(가장 새 것에 붙어 있다가 위로 올리면 그 자리), "↑ N개 더" 흐린 가장자리 |
| `sidebar_hints.c/h` | 아래 안내: 훅 켜기 제안(버튼, 실패 이유), 켜진 뒤에는 heron-limits 플러그인 안내. ×로 영영 닫는다 |
| `sidebar_hit.c/h` | 사이드바 위 마우스 대상과 툴팁 글 |
| `sidebar_input.c` | 사이드바 마우스와 키: hover(행이면 그 pane 테두리), 같은 대상에서 떼면 실행, 삭제 질문(다른 곳 누르기나 Esc로 닫기, 실패는 2.5초 표시), 휠 |
| `new_agent.c/h` | 새 에이전트: 기본 셸 pane을 열고 첫 프롬프트에 `claude`(또는 `claude --worktree`) 입력 |
| `folder_menu.c/h`, `folder_menu_layout.c/h`, `folder_menu_paint.c` | 새 에이전트 ▾ 폴더 메뉴: 폴더마다 이름과 경로, git 저장소면 worktree 칩(백그라운드로 확인), "폴더 선택…" |
| `folder_picker.c/h` | Windows 폴더 선택 창(IFileOpenDialog) |
| `usage_bar.c/h`, `usage_layout.h`, `usage_bar_paint.c` | 하단 사용량 바(64px): 5시간·주간 한도 미터(240px, 초기화까지 남은 시간), 에이전트별 컨텍스트 칩(누르면 그 pane으로). 보일 것이 없으면 숨고 pane이 자리를 차지한다 |
| `saved_work.c/h` | 지난 작업: 시작할 때 저장된 pane을 셸과 폴더 그대로 다시 열고(세션은 이어 열 수 있으면 resume, 아니면 새로), 수동 배치를 되살린다. 1초마다 바뀐 때만 저장 |
| `launch.c/h` | 한 번만 실행(Release만): 두 번째 실행은 폴더를 이미 떠 있는 창에 넘기고 끝난다. 창 핸들은 이름 붙은 공유 메모리로 찾는다(다른 빌드의 heron-c 창은 건드리지 않음) |

## 테스트 `tests/`

`heron-c-tests.exe`는 `base/`, `vt/`, `input/`, `layout/`, `claude/`, `agent/`, `hooks/`, `git/`, `buddy/`, `app/`만으로 빌드한다(창이 필요 없다).

| 경로 | 역할 |
| --- | --- |
| `check.h`, `test_main.c` | 작은 테스트 틀: `CHECK`가 실패를 세고 끝에서 개수를 출력 |
| `term_helpers.c/h` | C 문자열 넣기, 줄을 UTF-8로 읽기, 응답 꺼내기 |
| `test_vt_screen.c` | 출력, 줄바꿈, 넓은 글자, 나뉜 UTF-8, 스크롤백, 스크롤 영역, 대체 화면, 크기 변경, 선택 |
| `test_vt_codes.c` | 커서와 지우기, 색, 응답, 모드, OSC, DEC 선 문자와 탭 |
| `test_input.c` | 키, 마우스 보고, 붙여넣기 |
| `test_links.c` | URL 찾기, 셀 아래 링크(OSC 8, 여러 줄 URL) |
| `test_split.c` | 분할 트리 바꾸기, 픽셀 상자와 경계선, 놓을 자리 |
| `test_arrange.c` | 자동 격자, 자동과 수동 배치 전환, 경계선 끌기 최소 크기와 반반, 시간 곡선 |
| `test_json.c` | JSON 읽고 다시 쓰기(키 순서, 숫자 글자, 배열 안의 문자열), 이스케이프, 잘못된 글, 고치기 |
| `test_hooks.c` | 훅 항목 넣기와 빼기(사용자 훅과 Heron 훅은 그대로), 훅 이벤트별 상태, 모델, 서브에이전트 기록 |
| `test_agent.c` | 제목으로 본 상태, 훅 상태 적용, 사용 한도와 컨텍스트 |
| `test_answers.c` | 대화 기록의 답 읽기, 모델 확인, 기록 따라가기 |
| `test_sessions.c` | 세션 파일 끝에서 폴더와 제목 |
| `test_paths.c` | 경로 도우미, 세션 폴더 묶기, 시간 글 |
| `test_git.c` | numstat 합계와 새 파일 줄 수 |
| `test_workspace.c` | 배치 모양 저장과 확인, 지난 작업 형식 |
| `test_buddy_ground.c` | 그림 크기, 빈 칸 찾기(배경색, 반전, 커서, 가장 아래, 가장 가까운 곳, 아무 곳, 자리 없음) |
| `test_buddy_crew.c` | 왜가리의 기분별 몸짓과 프레임 요청, 덮이면 날아가기(동기화 출력 중에는 기다림), 새끼, 사라졌다 돌아오기 |

## 로그

- Debug 빌드(`build/heron-c.exe`)는 콘솔 프로그램이라 띄운 터미널에 로그가 나온다. 기본 레벨은 debug.
- `HERON_LOG`(error, warn, info, debug, trace)로 바꾼다. trace는 pty 읽기마다 바이트 수와 출력 내용을 이스케이프해서 찍는다.
- Release 빌드는 GUI 프로그램이고 기본 레벨은 warn이다.
- `--agent-hook`으로 실행될 때는 error만 남긴다(훅 출력은 대화에 섞일 수 있어서).
