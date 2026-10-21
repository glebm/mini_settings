#!/bin/sh

GET_BIN="$(dirname "$0")/../builddir/mini-settings-get"
SET_BIN="$(dirname "$0")/../builddir/mini-settings-set"
ORIG_CONF="$(dirname "$0")/test.conf"
CONF=/tmp/mini-settings-test.conf

ERRORS=""

# Arguments: <message>
add_error() {
  ERRORS="$ERRORS
$1"
}

# Arguments: <key> <expected>
expect_key() {
  ACTUAL="$("$GET_BIN" -f "$CONF" "$1")"
  if [ "$ACTUAL" != "$2" ]; then
    add_error "Expected $1 to be $2, got $ACTUAL"
  fi
}

# Arguments: <key> <value> ...
write_key() {
  "$SET_BIN" -f "$CONF" "$@" || add_error "write failed"
}

cp "$ORIG_CONF" "$CONF"

# Read existing key
expect_key kodi.enabled 1

# Read commented key
expect_key splash.screen.enabled ''

# Read key that does not exist
expect_key does.not.exist ''

# Update existing key
write_key kodi.enabled 2
expect_key kodi.enabled 2

# Uncomment key
write_key splash.screen.enabled 1
expect_key splash.screen.enabled 1

# Write an entirely new key
write_key totally.new.key 10
expect_key totally.new.key 10

# Write multiple keys
write_key multiple1 1 multiple2 2
expect_key multiple1 1
expect_key multiple2 2

# Read from a pipe
ACTUAL="$(printf 'piped=1\n' | "$GET_BIN" piped)"
[ "$ACTUAL" = 1 ] || add_error "Expected piped get to print 1, got $ACTUAL"
ACTUAL="$(printf 'piped=1\n' | "$SET_BIN" other 2)"
[ "$ACTUAL" = "piped=1
other=2" ] || add_error "Unexpected piped set output: $ACTUAL"

# Preserve file mode
chmod 640 "$CONF"
write_key mode.test 1
MODE="$(stat -c %a "$CONF")"
[ "$MODE" = 640 ] || add_error "Expected mode 640, got $MODE"

# Write through a symlink
LINK=/tmp/mini-settings-test-link.conf
ln -sf "$CONF" "$LINK"
"$SET_BIN" -f "$LINK" symlink.test 1 || add_error "write via symlink failed"
[ -L "$LINK" ] || add_error "Expected $LINK to remain a symlink"
expect_key symlink.test 1
rm -f "$LINK"

# Concurrent writes and reads (requires -Dconcurrency=true)
if [ "${TEST_CONCURRENCY:-1}" = 1 ]; then
  cp "$ORIG_CONF" "$CONF"
  READ_FAILURES=/tmp/mini-settings-test-read-failures
  : > "$READ_FAILURES"
  NUM_WRITERS=100

  i=0
  while [ $i -lt 4 ]; do
    (
      j=0
      while [ $j -lt 100 ]; do
        VALUE="$("$GET_BIN" -f "$CONF" kodi.enabled)"
        [ "$VALUE" = 1 ] || echo "read kodi.enabled=$VALUE" >> "$READ_FAILURES"
        j=$((j + 1))
      done
    ) &
    i=$((i + 1))
  done

  i=0
  while [ $i -lt $NUM_WRITERS ]; do
    "$SET_BIN" -f "$CONF" "concurrent.$i" "$i" &
    i=$((i + 1))
  done
  wait

  if [ -s "$READ_FAILURES" ]; then
    add_error "Concurrent reads saw a partial file:
$(sort "$READ_FAILURES" | uniq -c)"
  fi
  rm -f "$READ_FAILURES"

  i=0
  while [ $i -lt $NUM_WRITERS ]; do
    expect_key "concurrent.$i" "$i"
    i=$((i + 1))
  done
  expect_key kodi.enabled 1

  LEFTOVER="$(ls "$CONF".* 2>/dev/null)"
  [ -z "$LEFTOVER" ] || add_error "Leftover temporary files: $LEFTOVER"
fi

if [ -n "$ERRORS" ]; then
  echo >&2 "$ERRORS"
  exit 1
else
  echo >&2 OK
fi
