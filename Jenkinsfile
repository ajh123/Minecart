pipeline {
    agent any
    environment {
        REPO_URL = 'https://maven.minersonline.uk/snapshots'
        REPO_USER = credentials('maven-repo-username')
        REPO_PASSWORD = credentials('maven-repo-password')
        GITHUB_TOKEN = credentials('github-token') // Add GitHub token as a secret credential
    }
    stages {
        stage('Checkout') {
            steps {
                script {
                    // Use the token for authentication in GitHub URL
                    def gitUrl = "https://${GITHUB_TOKEN}@github.com/ajh123/Minecart.git"
                    git url: gitUrl, branch: 'main'
                }
            }
        }
        stage('Set Executable Permissions') {
            steps {
                // Ensure gradlew has the right permissions
                sh 'chmod +x ./gradlew'
            }
        }
        stage('Build with Gradle') {
            steps {
                sh './gradlew clean build --refresh-dependencies'
            }
        }
        stage('Publish Artifacts') {
            steps {
                sh "./gradlew publish"
            }
        }
    }
    post {
        success {
            echo 'Build and publish completed successfully.'
        }
        failure {
            echo 'Build or publish failed.'
        }
    }
}
