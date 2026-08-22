import { defineConfig } from 'vite';
import viteCompression from 'vite-plugin-compression';
import AutoImport from 'unplugin-auto-import/vite';
import Components from 'unplugin-vue-components/vite';
import { ElementPlusResolver } from 'unplugin-vue-components/resolvers';
import vue from '@vitejs/plugin-vue';
import { execFileSync } from 'node:child_process';
import { resolve } from 'path';

function runGit(args: string[]): string {
	try {
		return execFileSync('git', args, {
			cwd: import.meta.dirname,
			encoding: 'utf8',
			stdio: ['ignore', 'pipe', 'pipe'],
		}).trim();
	} catch (error) {
		throw new Error(`Unable to determine application version from Git: git ${args.join(' ')}`, {
			cause: error,
		});
	}
}

function tryRunGit(args: string[]): string | undefined {
	try {
		return execFileSync('git', args, {
			cwd: import.meta.dirname,
			encoding: 'utf8',
			stdio: ['ignore', 'pipe', 'ignore'],
		}).trim();
	} catch {
		return undefined;
	}
}

function getGitVersion(): string {
	const tag = runGit(['describe', '--tags', '--abbrev=0', 'HEAD']);
	const commitCount = Number(runGit(['rev-list', '--count', `${tag}..HEAD`]));
	const isExactTag = commitCount === 0;
	const branch = tryRunGit(['symbolic-ref', '--quiet', '--short', 'HEAD']);
	const isDirty = runGit(['status', '--porcelain', '--untracked-files=normal']).length > 0;

	let version = tag;

	if (!isExactTag) {
		version += `-${commitCount}-${runGit(['rev-parse', '--short', 'HEAD'])}`;
	}

	if (branch && branch !== 'main') {
		version += `+${branch}`;
	} else if (!branch && !isExactTag) {
		version += '+detached';
	}

	if (isDirty) {
		version += '-dirty';
	}

	return version;
}

const appVersion = getGitVersion();

// https://vite.dev/config/
export default defineConfig({
	define: {
		__APP_VERSION__: JSON.stringify(appVersion),
	},
	resolve: {
		alias: {
			'@': resolve(import.meta.dirname, 'src'),
			'@services': resolve(import.meta.dirname, 'src/services'),
			'@composables': resolve(import.meta.dirname, 'src/composables'),
			'@assets': resolve(import.meta.dirname, 'src/assets'),
			'@components': resolve(import.meta.dirname, 'src/components'),
			'@types': resolve(import.meta.dirname, 'src/types'),
			'@api': resolve(import.meta.dirname, 'src/api'),
			'@config': resolve(import.meta.dirname, 'src/config'),
			'@stores': resolve(import.meta.dirname, 'src/stores'),
			'@utils': resolve(import.meta.dirname, 'src/utils'),
		},
	},
	plugins: [
		viteCompression({
			algorithm: 'gzip',
			ext: '.gz',
			threshold: 10240,
			deleteOriginFile: true,
			filter: /\.(js|css|html|svg)$/,
			verbose: true,
		}),
		vue(),
		AutoImport({
			resolvers: [ElementPlusResolver()],
		}),
		Components({
			resolvers: [ElementPlusResolver()],
		}),
	],
	server: {
		watch: {
			usePolling: true,
		},
		host: '0.0.0.0',
		port: 5173,
	},
	build: {
		outDir: '../3rd_eyes/data/www',
		emptyOutDir: true,
	},
});
