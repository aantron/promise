const resaurus = require("resaurus");

const siteConfig = {
  title: 'Repromise',
  tagline: 'Type-safe JS promises for ReasonML',
  url: 'https://aantron.github.io/repromise',
  baseUrl: '/repromise/',

  projectName: 'repromise',
  organizationName: 'aantron',

  headerLinks: [
    {doc: 'QuickStart', label: 'Docs'},
    {doc: 'API', label: 'API'},
    {href: "https://github.com/aantron/repromise", label: "GitHub"},
  ],

  editUrl: "https://github.com/aantron/repromise/tree/master/doc/docs/",

  // This copyright info is used in /core/Footer.js and blog rss/atom feeds.
  copyright:
    'Copyright © ' +
    new Date().getFullYear() +
    ' Anton Bachin',

  /* On page navigation for the current documentation page */
  onPageNav: 'separate',

  // You may provide arbitrary config keys to be used as needed by your
  // template. For example, if you need your repo's URL...
  //   repoUrl: 'https://github.com/facebook/test-site',

  cleanUrl: true,
};

module.exports = resaurus(siteConfig);
