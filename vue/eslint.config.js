import js from '@eslint/js';
import typescript from '@typescript-eslint/eslint-plugin';
import typescriptParser from '@typescript-eslint/parser';
import pluginVue from 'eslint-plugin-vue';
import vueParser from 'vue-eslint-parser';
import prettier from 'eslint-plugin-prettier';

export default [
	// 忽略的文件
	{
		ignores: [
			'**/dist/**',
			'**/node_modules/**',
			'**/.vscode/**',
			'**/coverage/**',
			'**/*.config.js',
			'**/*.config.ts',
			'**/auto-imports.d.ts',
			'**/components.d.ts',
		],
	},

	// JavaScript 基礎配置
	{
		files: ['**/*.js', '**/*.mjs', '**/*.cjs'],
		...js.configs.recommended,
		rules: {
			'no-console': process.env.NODE_ENV === 'production' ? 'warn' : 'off',
			'no-debugger': process.env.NODE_ENV === 'production' ? 'error' : 'off',
		},
	},

	// TypeScript 配置
	{
		files: ['**/*.ts', '**/*.tsx'],
		languageOptions: {
			parser: typescriptParser,
			parserOptions: {
				ecmaVersion: 'latest',
				sourceType: 'module',
			},
		},
		plugins: {
			'@typescript-eslint': typescript,
		},
		rules: {
			'@typescript-eslint/no-explicit-any': 'warn',
			'@typescript-eslint/no-unused-vars': ['warn', { argsIgnorePattern: '^_' }],
			'@typescript-eslint/explicit-function-return-type': 'off',
			'@typescript-eslint/explicit-module-boundary-types': 'off',
			'@typescript-eslint/no-non-null-assertion': 'warn',
			'no-unused-vars': 'off',
			'no-undef': 'off',
		},
	},

	// Vue 配置
	...pluginVue.configs['flat/recommended'],
	{
		files: ['**/*.vue'],
		languageOptions: {
			parser: vueParser,
			parserOptions: {
				parser: typescriptParser,
				ecmaVersion: 'latest',
				sourceType: 'module',
			},
		},
		rules: {
			'vue/multi-word-component-names': 'off',
			'vue/no-v-html': 'warn',
			'vue/require-default-prop': 'off',
			'vue/require-explicit-emits': 'warn',
			'vue/html-indent': ['warn', 'tab'],
			'vue/max-attributes-per-line': 'off',
		},
	},

	// Prettier 配置
	{
		plugins: {
			prettier,
		},
		rules: {
			'prettier/prettier': 'warn',
		},
	},
];
