const {
  withPlugins,
  AndroidConfig,
  createRunOncePlugin,
} = require('@expo/config-plugins');

const { name, version } = require('./package.json');

const withWebAssembly = (config) => {
  if (!config.ios) config.ios = {};
  if (!config.ios.infoPlist) config.ios.infoPlist = {};

  const androidPermissions = [];

  return withPlugins(config, [
    [AndroidConfig.Permissions.withPermissions, androidPermissions],
  ]);
};

module.exports = createRunOncePlugin(withWebAssembly, name, version);
