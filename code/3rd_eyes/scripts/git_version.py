import json
import subprocess

from SCons.Errors import UserError

Import("env")

project_dir = env.subst("$PROJECT_DIR")


def git(*args, allow_failure=False):
	try:
		result = subprocess.run(
			["git", *args],
			cwd=project_dir,
			check=False,
			capture_output=True,
			text=True,
			encoding="utf-8",
		)
	except FileNotFoundError as error:
		raise UserError("Git executable was not found; firmware version cannot be generated") from error

	if result.returncode != 0 and not allow_failure:
		detail = result.stderr.strip() or result.stdout.strip() or "unknown Git error"
		raise UserError(f"Failed to generate firmware version: git {' '.join(args)}: {detail}")

	return result.stdout.strip() if result.returncode == 0 else None


if git("rev-parse", "--is-inside-work-tree") != "true":
	raise UserError("Project directory is not inside a Git work tree")

tag = git("describe", "--tags", "--abbrev=0", "HEAD")
commit_count = int(git("rev-list", "--count", f"{tag}..HEAD"))
exact_tag = commit_count == 0
branch = git("rev-parse", "--abbrev-ref", "HEAD")
short_sha = git("rev-parse", "--short", "HEAD")
dirty = bool(git("status", "--porcelain", "--untracked-files=normal"))

version = tag
if not exact_tag:
	version += f"-{commit_count}-{short_sha}"

if branch != "main":
	if branch == "HEAD":
		if not exact_tag:
			version += "+detached"
	else:
		version += f"+{branch}"

if dirty:
	version += "-dirty"

version_literal = json.dumps(version, ensure_ascii=True).replace('"', '\\"')
env.Append(CPPDEFINES=[("VERSION", version_literal)])
print(f"Firmware version: {version}")
