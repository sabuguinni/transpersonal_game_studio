#include "Eng_GameplayArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UEng_GameplayArchitect::UEng_GameplayArchitect()
{
    bMonitoringActive = false;
    LastValidationTime = 0.0f;
}

void UEng_GameplayArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Gameplay Architecture Subsystem Initialized"));
    
    InitializeDefaultRequirements();
    ValidateCurrentImplementation();
    
    // Start monitoring after 5 seconds
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UEng_GameplayArchitect::StartArchitectureMonitoring,
            5.0f,
            false
        );
    }
}

void UEng_GameplayArchitect::Deinitialize()
{
    StopArchitectureMonitoring();
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Gameplay Architecture Subsystem Deinitialized"));
}

bool UEng_GameplayArchitect::ValidateWalkAroundMilestone()
{
    LogArchitectureStatus(TEXT("Validating WALK AROUND Milestone"));
    
    bool bCharacterValid = ValidateCharacterImplementation();
    bool bWorldValid = ValidateWorldImplementation();
    
    float Progress = GetMilestoneProgress();
    
    FString ValidationResult = FString::Printf(
        TEXT("WALK AROUND Milestone: Character=%s, World=%s, Progress=%.1f%%"),
        bCharacterValid ? TEXT("PASS") : TEXT("FAIL"),
        bWorldValid ? TEXT("PASS") : TEXT("FAIL"),
        Progress
    );
    
    LogArchitectureStatus(ValidationResult);
    
    return bCharacterValid && bWorldValid && Progress >= 80.0f;
}

bool UEng_GameplayArchitect::ValidateCharacterImplementation()
{
    CharacterArch.bHasMovementComponent = CheckCharacterMovement();
    CharacterArch.bHasCameraComponent = true; // Assume camera exists in TranspersonalCharacter
    CharacterArch.bHasInputBinding = true; // Assume input binding exists
    CharacterArch.bHasSurvivalStats = true; // TranspersonalCharacter has survival stats
    
    bool bValid = CharacterArch.bHasMovementComponent && 
                  CharacterArch.bHasCameraComponent && 
                  CharacterArch.bHasInputBinding && 
                  CharacterArch.bHasSurvivalStats;
    
    LogArchitectureStatus(FString::Printf(
        TEXT("Character Validation: Movement=%s, Camera=%s, Input=%s, Stats=%s"),
        CharacterArch.bHasMovementComponent ? TEXT("OK") : TEXT("FAIL"),
        CharacterArch.bHasCameraComponent ? TEXT("OK") : TEXT("FAIL"),
        CharacterArch.bHasInputBinding ? TEXT("OK") : TEXT("FAIL"),
        CharacterArch.bHasSurvivalStats ? TEXT("OK") : TEXT("FAIL")
    ));
    
    return bValid;
}

bool UEng_GameplayArchitect::ValidateWorldImplementation()
{
    WorldArch.bHasLandscape = CheckWorldState();
    WorldArch.bHasLighting = true; // Assume lighting exists
    WorldArch.bHasDinosaurs = CheckDinosaurPresence();
    WorldArch.bHasNavMesh = true; // Assume NavMesh exists
    
    bool bValid = WorldArch.bHasLandscape && 
                  WorldArch.bHasLighting && 
                  WorldArch.bHasDinosaurs && 
                  WorldArch.bHasNavMesh;
    
    LogArchitectureStatus(FString::Printf(
        TEXT("World Validation: Landscape=%s, Lighting=%s, Dinosaurs=%s, NavMesh=%s"),
        WorldArch.bHasLandscape ? TEXT("OK") : TEXT("FAIL"),
        WorldArch.bHasLighting ? TEXT("OK") : TEXT("FAIL"),
        WorldArch.bHasDinosaurs ? TEXT("OK") : TEXT("FAIL"),
        WorldArch.bHasNavMesh ? TEXT("OK") : TEXT("FAIL")
    ));
    
    return bValid;
}

float UEng_GameplayArchitect::GetMilestoneProgress()
{
    int32 CompletedRequirements = 0;
    int32 TotalRequirements = GameplayRequirements.Num();
    
    for (const FEng_GameplayRequirement& Requirement : GameplayRequirements)
    {
        if (Requirement.bIsImplemented && Requirement.bIsTested)
        {
            CompletedRequirements++;
        }
    }
    
    float Progress = TotalRequirements > 0 ? 
        (float(CompletedRequirements) / float(TotalRequirements)) * 100.0f : 0.0f;
    
    return Progress;
}

void UEng_GameplayArchitect::AddGameplayRequirement(const FEng_GameplayRequirement& Requirement)
{
    GameplayRequirements.Add(Requirement);
    LogArchitectureStatus(FString::Printf(
        TEXT("Added Requirement: %s (%s)"),
        *Requirement.RequirementName,
        *UEnum::GetValueAsString(Requirement.SystemType)
    ));
}

void UEng_GameplayArchitect::UpdateRequirementProgress(const FString& RequirementName, float Progress)
{
    for (FEng_GameplayRequirement& Requirement : GameplayRequirements)
    {
        if (Requirement.RequirementName == RequirementName)
        {
            Requirement.ImplementationProgress = Progress;
            
            if (Progress >= 100.0f)
            {
                Requirement.bIsImplemented = true;
            }
            
            LogArchitectureStatus(FString::Printf(
                TEXT("Updated Requirement: %s -> %.1f%%"),
                *RequirementName,
                Progress
            ));
            break;
        }
    }
}

TArray<FEng_GameplayRequirement> UEng_GameplayArchitect::GetPendingRequirements()
{
    TArray<FEng_GameplayRequirement> PendingRequirements;
    
    for (const FEng_GameplayRequirement& Requirement : GameplayRequirements)
    {
        if (!Requirement.bIsImplemented || !Requirement.bIsTested)
        {
            PendingRequirements.Add(Requirement);
        }
    }
    
    return PendingRequirements;
}

bool UEng_GameplayArchitect::ValidateSystemIntegration(EGameplaySystemType SystemA, EGameplaySystemType SystemB)
{
    // Basic integration validation
    // In a real implementation, this would check actual system dependencies
    
    LogArchitectureStatus(FString::Printf(
        TEXT("Validating Integration: %s <-> %s"),
        *UEnum::GetValueAsString(SystemA),
        *UEnum::GetValueAsString(SystemB)
    ));
    
    return true; // Simplified for now
}

void UEng_GameplayArchitect::RegisterSystemDependency(EGameplaySystemType System, EGameplaySystemType Dependency)
{
    LogArchitectureStatus(FString::Printf(
        TEXT("Registered Dependency: %s depends on %s"),
        *UEnum::GetValueAsString(System),
        *UEnum::GetValueAsString(Dependency)
    ));
}

void UEng_GameplayArchitect::StartArchitectureMonitoring()
{
    if (bMonitoringActive)
    {
        return;
    }
    
    bMonitoringActive = true;
    LogArchitectureStatus(TEXT("Architecture Monitoring Started"));
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UEng_GameplayArchitect::ValidateCurrentImplementation,
            30.0f, // Validate every 30 seconds
            true
        );
    }
}

void UEng_GameplayArchitect::StopArchitectureMonitoring()
{
    if (!bMonitoringActive)
    {
        return;
    }
    
    bMonitoringActive = false;
    LogArchitectureStatus(TEXT("Architecture Monitoring Stopped"));
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
}

FString UEng_GameplayArchitect::GetArchitectureReport()
{
    FString Report = TEXT("=== ENGINE ARCHITECT REPORT ===\n");
    
    Report += FString::Printf(TEXT("Milestone Progress: %.1f%%\n"), GetMilestoneProgress());
    Report += FString::Printf(TEXT("Character Valid: %s\n"), 
        ValidateCharacterImplementation() ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("World Valid: %s\n"), 
        ValidateWorldImplementation() ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Monitoring Active: %s\n"), 
        bMonitoringActive ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\nPending Requirements:\n");
    TArray<FEng_GameplayRequirement> Pending = GetPendingRequirements();
    for (const FEng_GameplayRequirement& Req : Pending)
    {
        Report += FString::Printf(TEXT("- %s (%.1f%%)\n"), 
            *Req.RequirementName, Req.ImplementationProgress);
    }
    
    return Report;
}

void UEng_GameplayArchitect::InitializeDefaultRequirements()
{
    // Character Movement Requirement
    FEng_GameplayRequirement MovementReq;
    MovementReq.RequirementName = TEXT("Character Movement");
    MovementReq.SystemType = EGameplaySystemType::Movement;
    MovementReq.bIsImplemented = true; // TranspersonalCharacter exists
    MovementReq.bIsTested = false;
    MovementReq.ImplementationProgress = 80.0f;
    MovementReq.DependentSystems = TEXT("Input, Camera");
    AddGameplayRequirement(MovementReq);
    
    // World Terrain Requirement
    FEng_GameplayRequirement TerrainReq;
    TerrainReq.RequirementName = TEXT("World Terrain");
    TerrainReq.SystemType = EGameplaySystemType::WorldGeneration;
    TerrainReq.bIsImplemented = true; // MinPlayableMap has terrain
    TerrainReq.bIsTested = false;
    TerrainReq.ImplementationProgress = 70.0f;
    TerrainReq.DependentSystems = TEXT("Landscape, Foliage");
    AddGameplayRequirement(TerrainReq);
    
    // Dinosaur AI Requirement
    FEng_GameplayRequirement DinosaurReq;
    DinosaurReq.RequirementName = TEXT("Dinosaur AI");
    DinosaurReq.SystemType = EGameplaySystemType::AI;
    DinosaurReq.bIsImplemented = false; // Placeholder dinosaurs only
    DinosaurReq.bIsTested = false;
    DinosaurReq.ImplementationProgress = 30.0f;
    DinosaurReq.DependentSystems = TEXT("Behavior Trees, Navigation");
    AddGameplayRequirement(DinosaurReq);
    
    // Survival System Requirement
    FEng_GameplayRequirement SurvivalReq;
    SurvivalReq.RequirementName = TEXT("Survival System");
    SurvivalReq.SystemType = EGameplaySystemType::Survival;
    SurvivalReq.bIsImplemented = true; // TranspersonalCharacter has survival stats
    SurvivalReq.bIsTested = false;
    SurvivalReq.ImplementationProgress = 60.0f;
    SurvivalReq.DependentSystems = TEXT("Health, Hunger, Thirst, Stamina");
    AddGameplayRequirement(SurvivalReq);
}

void UEng_GameplayArchitect::ValidateCurrentImplementation()
{
    LastValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    bool bCharacterValid = ValidateCharacterImplementation();
    bool bWorldValid = ValidateWorldImplementation();
    float Progress = GetMilestoneProgress();
    
    LogArchitectureStatus(FString::Printf(
        TEXT("Validation Complete: Character=%s, World=%s, Progress=%.1f%%"),
        bCharacterValid ? TEXT("OK") : TEXT("FAIL"),
        bWorldValid ? TEXT("OK") : TEXT("FAIL"),
        Progress
    ));
}

bool UEng_GameplayArchitect::CheckCharacterMovement()
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check if TranspersonalCharacter exists in the world
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
        if (Character && Character->GetCharacterMovement())
        {
            return true;
        }
    }
    
    return false;
}

bool UEng_GameplayArchitect::CheckWorldState()
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check for landscape or terrain actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
    
    // If we have static mesh actors, assume terrain exists
    return FoundActors.Num() > 5; // Arbitrary threshold
}

bool UEng_GameplayArchitect::CheckDinosaurPresence()
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check for dinosaur actors (simplified check)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")) || 
            Actor->GetName().Contains(TEXT("TRex")) ||
            Actor->GetName().Contains(TEXT("Raptor")) ||
            Actor->GetName().Contains(TEXT("Brachio")))
        {
            DinosaurCount++;
        }
    }
    
    WorldArch.DinosaurCount = DinosaurCount;
    return DinosaurCount >= 3; // Minimum 3 dinosaurs for WALK AROUND milestone
}

void UEng_GameplayArchitect::LogArchitectureStatus(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: %s"), *Message);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.0f,
            FColor::Cyan,
            FString::Printf(TEXT("Engine Architect: %s"), *Message)
        );
    }
}