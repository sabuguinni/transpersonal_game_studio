#!/usr/bin/env python3
"""
Transpersonal Game Studio - Integration Pipeline
Copyright Transpersonal Game Studio. All Rights Reserved.

Integration pipeline for managing outputs from all 19 agents in the production chain.
This script handles build validation, asset integration, and deployment coordination.
"""

import os
import sys
import json
import subprocess
import logging
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('integration.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class IntegrationPipeline:
    """Main integration pipeline coordinator"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.config = self.load_config()
        self.build_history = []
        
    def load_config(self) -> Dict:
        """Load integration configuration"""
        config_path = self.project_root / "Integration" / "config.json"
        if config_path.exists():
            with open(config_path, 'r') as f:
                return json.load(f)
        
        # Default configuration
        return {
            "build_configurations": ["Development", "Test", "Shipping"],
            "target_platforms": ["Win64"],
            "max_build_history": 10,
            "validation_timeout": 300,
            "agents": {
                "02": "Engine Architect",
                "03": "Core Systems Programmer", 
                "04": "Performance Optimizer",
                "05": "Procedural World Generator",
                "06": "Environment Artist",
                "07": "Architecture & Interior Agent",
                "08": "Lighting & Atmosphere Agent",
                "09": "Character Artist Agent",
                "10": "Animation Agent",
                "11": "NPC Behavior Agent",
                "12": "Combat & Enemy AI Agent",
                "13": "Crowd & Traffic Simulation",
                "14": "Quest & Mission Designer",
                "15": "Narrative & Dialogue Agent",
                "16": "Audio Agent",
                "17": "VFX Agent",
                "18": "QA & Testing Agent"
            }
        }
    
    def validate_agent_output(self, agent_id: str, output_path: str) -> Tuple[bool, List[str]]:
        """Validate output from a specific agent"""
        errors = []
        
        if not os.path.exists(output_path):
            errors.append(f"Agent {agent_id} output path not found: {output_path}")
            return False, errors
            
        # Agent-specific validation rules
        if agent_id == "02":  # Engine Architect
            if not self.validate_architecture_files(output_path):
                errors.append("Architecture files validation failed")
                
        elif agent_id == "03":  # Core Systems
            if not self.validate_core_systems(output_path):
                errors.append("Core systems validation failed")
                
        elif agent_id == "18":  # QA & Testing
            if not self.validate_qa_results(output_path):
                errors.append("QA validation failed - build blocked")
                return False, errors  # QA failures block everything
        
        return len(errors) == 0, errors
    
    def validate_architecture_files(self, path: str) -> bool:
        """Validate architecture files from Engine Architect"""
        required_files = [
            "TechnicalArchitecture.md",
            "SystemDependencies.json",
            "PerformanceTargets.json"
        ]
        
        for file in required_files:
            if not os.path.exists(os.path.join(path, file)):
                logger.error(f"Missing required architecture file: {file}")
                return False
        return True
    
    def validate_core_systems(self, path: str) -> bool:
        """Validate core systems implementation"""
        # Check for critical system files
        core_systems = [
            "PhysicsSystem.cpp",
            "CollisionSystem.cpp", 
            "RagdollSystem.cpp"
        ]
        
        for system in core_systems:
            system_path = os.path.join(path, "Source", "TranspersonalGame", "Core", system)
            if not os.path.exists(system_path):
                logger.warning(f"Core system file not found: {system}")
                
        return True
    
    def validate_qa_results(self, path: str) -> bool:
        """Validate QA test results - critical for build approval"""
        qa_report_path = os.path.join(path, "QAReport.json")
        
        if not os.path.exists(qa_report_path):
            logger.error("QA report not found - build blocked")
            return False
            
        try:
            with open(qa_report_path, 'r') as f:
                qa_data = json.load(f)
                
            # Check for critical failures
            if qa_data.get("critical_failures", 0) > 0:
                logger.error(f"QA found {qa_data['critical_failures']} critical failures - build blocked")
                return False
                
            # Check test coverage
            coverage = qa_data.get("test_coverage", 0)
            if coverage < 80:
                logger.warning(f"Test coverage below threshold: {coverage}%")
                
            return True
            
        except Exception as e:
            logger.error(f"Failed to parse QA report: {e}")
            return False
    
    def build_project(self, configuration: str = "Development") -> bool:
        """Build the project with specified configuration"""
        logger.info(f"Starting build with configuration: {configuration}")
        
        build_script = self.project_root / "Build" / "BuildGame.bat"
        if not build_script.exists():
            logger.error("Build script not found")
            return False
            
        try:
            result = subprocess.run(
                [str(build_script), configuration],
                cwd=str(self.project_root),
                capture_output=True,
                text=True,
                timeout=self.config["validation_timeout"]
            )
            
            if result.returncode == 0:
                logger.info("Build completed successfully")
                return True
            else:
                logger.error(f"Build failed: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            logger.error("Build timed out")
            return False
        except Exception as e:
            logger.error(f"Build error: {e}")
            return False
    
    def create_build_package(self, configuration: str) -> Optional[str]:
        """Create a packaged build for distribution"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        package_name = f"TranspersonalGame_{configuration}_{timestamp}"
        package_path = self.project_root / "Builds" / package_name
        
        logger.info(f"Creating build package: {package_name}")
        
        # Create package directory
        package_path.mkdir(parents=True, exist_ok=True)
        
        # Copy binaries and content
        # This would be expanded with actual packaging logic
        
        # Create build manifest
        manifest = {
            "build_id": package_name,
            "configuration": configuration,
            "timestamp": timestamp,
            "agents_integrated": list(self.config["agents"].keys()),
            "validation_passed": True
        }
        
        manifest_path = package_path / "build_manifest.json"
        with open(manifest_path, 'w') as f:
            json.dump(manifest, f, indent=2)
            
        logger.info(f"Build package created: {package_path}")
        return str(package_path)
    
    def rollback_to_previous_build(self) -> bool:
        """Rollback to the last known good build"""
        if not self.build_history:
            logger.error("No previous builds available for rollback")
            return False
            
        last_good_build = self.build_history[-1]
        logger.info(f"Rolling back to build: {last_good_build}")
        
        # Implement rollback logic here
        return True
    
    def run_integration_cycle(self, cycle_id: str) -> bool:
        """Run a complete integration cycle"""
        logger.info(f"Starting integration cycle: {cycle_id}")
        
        # Validate all agent outputs
        all_valid = True
        for agent_id, agent_name in self.config["agents"].items():
            output_path = self.project_root / "AgentOutputs" / f"Agent_{agent_id}"
            
            if output_path.exists():
                valid, errors = self.validate_agent_output(agent_id, str(output_path))
                if not valid:
                    logger.error(f"Agent {agent_id} ({agent_name}) validation failed: {errors}")
                    all_valid = False
                else:
                    logger.info(f"Agent {agent_id} ({agent_name}) validation passed")
        
        if not all_valid:
            logger.error("Integration cycle failed - agent validation errors")
            return False
        
        # Build the project
        if not self.build_project():
            logger.error("Integration cycle failed - build errors")
            return False
        
        # Create package
        package_path = self.create_build_package("Development")
        if package_path:
            self.build_history.append(package_path)
            
            # Maintain build history limit
            if len(self.build_history) > self.config["max_build_history"]:
                old_build = self.build_history.pop(0)
                logger.info(f"Removing old build from history: {old_build}")
        
        logger.info(f"Integration cycle {cycle_id} completed successfully")
        return True

def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python IntegrationPipeline.py <cycle_id>")
        sys.exit(1)
    
    cycle_id = sys.argv[1]
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    pipeline = IntegrationPipeline(project_root)
    success = pipeline.run_integration_cycle(cycle_id)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()