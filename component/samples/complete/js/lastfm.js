'use strict';

function _lastfm() {
	this.notify_data = (name, data) => {
		if (name == '2K3.NOTIFY.LASTFM' && data == 'update') {
			this.username = this.read_ini('username');
			this.api_key = this.read_ini('api_key');

			_.forEach(panel.text_objects, (item) => {
				if (item.mode == 'lastfm_bio') {
					item.artist = '';
					panel.item_focus_change();
				}
			});

			_.forEach(panel.list_objects, (item) => {
				if (item.mode == 'lastfm_info') {
					item.reset();
				}
			});
		}
	}

	this.update_username = () => {
		const username = utils.InputBox(window.ID, 'Enter your Last.fm username', window.ScriptInfo.Name, this.username);

		if (username != this.username) {
			this.write_ini('username', username);
			window.NotifyOthers('2K3.NOTIFY.LASTFM', 'update');
			this.notify_data('2K3.NOTIFY.LASTFM', 'update');
		}
	}

	this.update_api_key = () => {
		const api_key = utils.InputBox(window.ID, 'Enter your Last.fm API key', window.ScriptInfo.Name, this.api_key);

		if (api_key != this.api_key) {
			this.write_ini('api_key', api_key);
			window.NotifyOthers('2K3.NOTIFY.LASTFM', 'update');
			this.notify_data('2K3.NOTIFY.LASTFM', 'update');
		}
	}

	this.get_base_url = () => {
		return 'http://ws.audioscrobbler.com/2.0/?format=json&api_key=' + this.api_key;
	}

	this.read_ini = (k) => {
		return utils.ReadINI(this.ini_file, 'Last.fm', k);
	}

	this.write_ini = (k, v) => {
		utils.WriteINI(this.ini_file, 'Last.fm', k, v);
	}

	utils.CreateFolder(folders.data);
	this.ini_file = folders.data + 'lastfm.ini';
	this.username = this.read_ini('username');
	this.api_key = this.read_ini('api_key');
	this.ua = 'spider_monkey_panel_lastfm';
}
