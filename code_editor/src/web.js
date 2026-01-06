'use strict';

const path = require('path');
const webpack = require('webpack');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');
const ForkTsCheckerWebpackPlugin = require('fork-ts-checker-webpack-plugin');
const CircularDependencyPlugin = require('circular-dependency-plugin');

/**@type {import('webpack').Configuration}*/
const config = {
  target: 'node', 

  mode: 'production',

  entry: './extension.ts', 
  output: {
    
    path: path.resolve(__dirname, 'out'),
    filename: 'extension.js',
    libraryTarget: 'commonjs2',
    devtoolModuleFilenameTemplate: '../[resource-path]',
  },
