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
    {search: true},
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

  algolia: {
    apiKey: "e0a7347e16ff47590833503d435facf2",
    indexName: "repromise",
  },
};

module.exports = resaurus(siteConfig);
