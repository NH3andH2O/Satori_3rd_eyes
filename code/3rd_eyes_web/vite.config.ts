import { defineConfig } from 'vite';
import viteCompression from 'vite-plugin-compression';
import AutoImport from 'unplugin-auto-import/vite';
import Components from 'unplugin-vue-components/vite';
import { ElementPlusResolver } from 'unplugin-vue-components/resolvers';
import vue from '@vitejs/plugin-vue';
import { resolve } from 'path';

// https://vite.dev/config/
export default defineConfig({
	resolve: {
		alias: {
			'@': resolve(__dirname, 'src'),
			'@services': resolve(__dirname, 'src/services'),
			'@composables': resolve(__dirname, 'src/composables'),
			'@assets': resolve(__dirname, 'src/assets'),
			'@components': resolve(__dirname, 'src/components'),
			'@types': resolve(__dirname, 'src/types'),
			'@api': resolve(__dirname, 'src/api'),
			'@config': resolve(__dirname, 'src/config'),
			'@stores': resolve(__dirname, 'src/stores'),
			'@utils': resolve(__dirname, 'src/utils'),
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
		emptyOutDir: true
	},
});
