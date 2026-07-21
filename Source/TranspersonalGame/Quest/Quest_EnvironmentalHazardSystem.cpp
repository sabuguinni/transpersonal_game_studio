#include "Quest_EnvironmentalHazardSystem.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AQuest_EnvironmentalHazardSystem::AQuest_EnvironmentalHazardSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create hazard detection sphere
    HazardDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HazardDetectionSphere"));
    RootComponent = HazardDetectionSphere;
    HazardDetectionSphere->SetSphereRadius(2000.0f);
    HazardDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HazardDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    HazardDetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize response options
    InitializeResponseOptions();
}

void AQuest_EnvironmentalHazardSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Start environmental monitoring by default
    StartEnvironmentalMonitoring();
}

void AQuest_EnvironmentalHazardSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bHazardActive)
    {
        HazardTimeRemaining -= DeltaTime;
        if (HazardTimeRemaining <= 0.0f)
        {
            EndHazard();
        }
    }
}

void AQuest_EnvironmentalHazardSystem::TriggerHazard(EQuest_HazardType HazardType, FVector Location, EQuest_HazardSeverity Severity)
{
    if (bHazardActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger new hazard - one already active"));
        return;
    }

    CurrentHazard.HazardType = HazardType;
    CurrentHazard.Severity = Severity;
    CurrentHazard.EpicenterLocation = Location;

    // Configure hazard parameters based on type and severity
    switch (HazardType)
    {
        case EQuest_HazardType::VolcanicEruption:
            CurrentHazard.Duration = 120.0f + (float)Severity * 60.0f;
            CurrentHazard.AffectedRadius = 1500.0f + (float)Severity * 500.0f;
            CurrentHazard.DamagePerSecond = 15.0f + (float)Severity * 10.0f;
            CurrentHazard.bRequiresEvacuation = true;
            CurrentHazard.SurvivalTips = {"Move away from lava flows", "Seek underground shelter", "Cover mouth and nose"};
            break;

        case EQuest_HazardType::FlashFlood:
            CurrentHazard.Duration = 90.0f + (float)Severity * 30.0f;
            CurrentHazard.AffectedRadius = 2000.0f + (float)Severity * 1000.0f;
            CurrentHazard.DamagePerSecond = 20.0f + (float)Severity * 15.0f;
            CurrentHazard.bRequiresEvacuation = true;
            CurrentHazard.SurvivalTips = {"Get to higher ground immediately", "Avoid crossing flowing water", "Stay away from valleys"};
            break;

        case EQuest_HazardType::Wildfire:
            CurrentHazard.Duration = 180.0f + (float)Severity * 120.0f;
            CurrentHazard.AffectedRadius = 3000.0f + (float)Severity * 2000.0f;
            CurrentHazard.DamagePerSecond = 25.0f + (float)Severity * 20.0f;
            CurrentHazard.bRequiresEvacuation = true;
            CurrentHazard.SurvivalTips = {"Create firebreaks", "Move perpendicular to wind", "Find water source"};
            break;

        case EQuest_HazardType::Earthquake:
            CurrentHazard.Duration = 30.0f + (float)Severity * 15.0f;
            CurrentHazard.AffectedRadius = 5000.0f + (float)Severity * 3000.0f;
            CurrentHazard.DamagePerSecond = 30.0f + (float)Severity * 25.0f;
            CurrentHazard.bRequiresEvacuation = false;
            CurrentHazard.SurvivalTips = {"Drop, cover, and hold on", "Stay away from cliffs", "Avoid unstable structures"};
            break;

        case EQuest_HazardType::ToxicGas:
            CurrentHazard.Duration = 60.0f + (float)Severity * 40.0f;
            CurrentHazard.AffectedRadius = 800.0f + (float)Severity * 400.0f;
            CurrentHazard.DamagePerSecond = 12.0f + (float)Severity * 8.0f;
            CurrentHazard.bRequiresEvacuation = true;
            CurrentHazard.SurvivalTips = {"Move to higher ground", "Cover airways", "Exit gas cloud quickly"};
            break;

        case EQuest_HazardType::Stampede:
            CurrentHazard.Duration = 45.0f + (float)Severity * 30.0f;
            CurrentHazard.AffectedRadius = 1200.0f + (float)Severity * 800.0f;
            CurrentHazard.DamagePerSecond = 35.0f + (float)Severity * 30.0f;
            CurrentHazard.bRequiresEvacuation = true;
            CurrentHazard.SurvivalTips = {"Get behind solid cover", "Climb to safety", "Don't run with the herd"};
            break;

        default:
            CurrentHazard.Duration = 60.0f;
            CurrentHazard.AffectedRadius = 1000.0f;
            CurrentHazard.DamagePerSecond = 10.0f;
            CurrentHazard.bRequiresEvacuation = false;
            break;
    }

    bHazardActive = true;
    HazardTimeRemaining = CurrentHazard.Duration;

    // Update hazard detection sphere
    HazardDetectionSphere->SetSphereRadius(CurrentHazard.AffectedRadius);
    SetActorLocation(CurrentHazard.EpicenterLocation);

    // Start hazard update timer
    GetWorldTimerManager().SetTimer(HazardUpdateTimer, this, &AQuest_EnvironmentalHazardSystem::UpdateHazard, 1.0f, true);

    // Notify players
    NotifyPlayersOfHazard();

    // Generate emergency quests
    GenerateHazardSurvivalQuest();
    if (CurrentHazard.Severity >= EQuest_HazardSeverity::High)
    {
        GenerateHazardRescueQuest();
    }

    UE_LOG(LogTemp, Warning, TEXT("Environmental hazard triggered: %s at severity %d"), 
           *UEnum::GetValueAsString(HazardType), (int32)Severity);
}

void AQuest_EnvironmentalHazardSystem::EndHazard()
{
    if (!bHazardActive)
    {
        return;
    }

    bHazardActive = false;
    HazardTimeRemaining = 0.0f;

    // Clear timers
    GetWorldTimerManager().ClearTimer(HazardUpdateTimer);

    // Reset detection sphere
    HazardDetectionSphere->SetSphereRadius(2000.0f);

    UE_LOG(LogTemp, Log, TEXT("Environmental hazard ended: %s"), 
           *UEnum::GetValueAsString(CurrentHazard.HazardType));

    // Reset hazard data
    CurrentHazard = FQuest_HazardData();
}

void AQuest_EnvironmentalHazardSystem::AttemptHazardResponse(const FString& ResponseName)
{
    if (!bHazardActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active hazard to respond to"));
        return;
    }

    // Find the response
    FQuest_HazardResponse* Response = AvailableResponses.FindByPredicate([&ResponseName](const FQuest_HazardResponse& R) {
        return R.ResponseName == ResponseName;
    });

    if (!Response)
    {
        UE_LOG(LogTemp, Warning, TEXT("Response not found: %s"), *ResponseName);
        return;
    }

    // Calculate success based on response chance and hazard severity
    float SeverityModifier = 1.0f - ((float)CurrentHazard.Severity * 0.15f);
    float FinalSuccessChance = Response->SuccessChance * SeverityModifier;
    
    bool bSuccess = FMath::RandRange(0.0f, 1.0f) <= FinalSuccessChance;

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Hazard response successful: %s"), *ResponseName);
        
        // Reduce hazard impact or duration
        if (ResponseName.Contains("Evacuation"))
        {
            CurrentHazard.DamagePerSecond *= 0.3f; // Greatly reduce damage
        }
        else if (ResponseName.Contains("Shelter"))
        {
            CurrentHazard.DamagePerSecond *= 0.5f; // Moderate damage reduction
        }
        else if (ResponseName.Contains("Firebreak"))
        {
            HazardTimeRemaining *= 0.7f; // Reduce duration
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Hazard response failed: %s"), *ResponseName);
        // Failure might increase danger or consume resources without benefit
    }
}

TArray<FQuest_HazardResponse> AQuest_EnvironmentalHazardSystem::GetAvailableResponses() const
{
    TArray<FQuest_HazardResponse> FilteredResponses;

    for (const FQuest_HazardResponse& Response : AvailableResponses)
    {
        // Filter responses based on current hazard type
        bool bApplicable = false;

        if (CurrentHazard.HazardType == EQuest_HazardType::VolcanicEruption && 
            (Response.ResponseName.Contains("Evacuation") || Response.ResponseName.Contains("Underground")))
        {
            bApplicable = true;
        }
        else if (CurrentHazard.HazardType == EQuest_HazardType::FlashFlood && 
                 (Response.ResponseName.Contains("Higher Ground") || Response.ResponseName.Contains("Evacuation")))
        {
            bApplicable = true;
        }
        else if (CurrentHazard.HazardType == EQuest_HazardType::Wildfire && 
                 (Response.ResponseName.Contains("Firebreak") || Response.ResponseName.Contains("Water")))
        {
            bApplicable = true;
        }
        else if (Response.ResponseName.Contains("Shelter") || Response.ResponseName.Contains("First Aid"))
        {
            bApplicable = true; // Universal responses
        }

        if (bApplicable)
        {
            FilteredResponses.Add(Response);
        }
    }

    return FilteredResponses;
}

void AQuest_EnvironmentalHazardSystem::StartEnvironmentalMonitoring()
{
    if (bMonitoringActive)
    {
        return;
    }

    bMonitoringActive = true;
    GetWorldTimerManager().SetTimer(MonitoringTimer, this, &AQuest_EnvironmentalHazardSystem::CheckForEnvironmentalTriggers, MonitoringInterval, true);
    
    UE_LOG(LogTemp, Log, TEXT("Environmental monitoring started"));
}

void AQuest_EnvironmentalHazardSystem::StopEnvironmentalMonitoring()
{
    if (!bMonitoringActive)
    {
        return;
    }

    bMonitoringActive = false;
    GetWorldTimerManager().ClearTimer(MonitoringTimer);
    
    UE_LOG(LogTemp, Log, TEXT("Environmental monitoring stopped"));
}

void AQuest_EnvironmentalHazardSystem::GenerateHazardSurvivalQuest()
{
    if (!bHazardActive)
    {
        return;
    }

    // Create a survival quest based on the current hazard
    FString QuestName = FString::Printf(TEXT("Survive %s"), *UEnum::GetValueAsString(CurrentHazard.HazardType));
    FString QuestDescription = FString::Printf(TEXT("A %s has struck the area. You must survive for %d seconds while avoiding damage."), 
                                               *UEnum::GetValueAsString(CurrentHazard.HazardType), 
                                               (int32)CurrentHazard.Duration);

    UE_LOG(LogTemp, Log, TEXT("Generated hazard survival quest: %s"), *QuestName);
}

void AQuest_EnvironmentalHazardSystem::GenerateHazardRescueQuest()
{
    if (!bHazardActive || CurrentHazard.Severity < EQuest_HazardSeverity::High)
    {
        return;
    }

    // Create a rescue quest for high-severity hazards
    FString QuestName = FString::Printf(TEXT("Rescue Operation: %s"), *UEnum::GetValueAsString(CurrentHazard.HazardType));
    FString QuestDescription = TEXT("Other survivors are trapped in the hazard zone. Navigate the dangers and bring them to safety.");

    UE_LOG(LogTemp, Log, TEXT("Generated hazard rescue quest: %s"), *QuestName);
}

void AQuest_EnvironmentalHazardSystem::UpdateHazard()
{
    if (!bHazardActive)
    {
        return;
    }

    ProcessHazardDamage();
    
    // Update hazard intensity over time
    float TimeProgress = 1.0f - (HazardTimeRemaining / CurrentHazard.Duration);
    
    // Some hazards get worse over time, others diminish
    if (CurrentHazard.HazardType == EQuest_HazardType::Wildfire || 
        CurrentHazard.HazardType == EQuest_HazardType::FlashFlood)
    {
        // These hazards intensify then diminish
        float IntensityMultiplier = FMath::Sin(TimeProgress * PI); // Bell curve
        CurrentHazard.DamagePerSecond = CurrentHazard.DamagePerSecond * IntensityMultiplier;
    }
}

void AQuest_EnvironmentalHazardSystem::CheckForEnvironmentalTriggers()
{
    if (bHazardActive)
    {
        return; // Don't trigger new hazards while one is active
    }

    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    if (RandomValue <= HazardProbability)
    {
        // Randomly select a hazard type
        TArray<EQuest_HazardType> PossibleHazards = {
            EQuest_HazardType::VolcanicEruption,
            EQuest_HazardType::FlashFlood,
            EQuest_HazardType::Wildfire,
            EQuest_HazardType::Earthquake,
            EQuest_HazardType::ToxicGas,
            EQuest_HazardType::Stampede
        };

        EQuest_HazardType SelectedHazard = PossibleHazards[FMath::RandRange(0, PossibleHazards.Num() - 1)];
        EQuest_HazardSeverity SelectedSeverity = (EQuest_HazardSeverity)FMath::RandRange(0, 4);

        // Generate random location within reasonable range
        FVector RandomLocation = GetActorLocation() + FVector(
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-5000.0f, 5000.0f),
            0.0f
        );

        TriggerHazard(SelectedHazard, RandomLocation, SelectedSeverity);
    }
}

void AQuest_EnvironmentalHazardSystem::ProcessHazardDamage()
{
    // This would typically damage players/NPCs within the hazard zone
    // For now, just log the damage processing
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing hazard damage: %.1f DPS over %.0f radius"), 
           CurrentHazard.DamagePerSecond, CurrentHazard.AffectedRadius);
}

void AQuest_EnvironmentalHazardSystem::NotifyPlayersOfHazard()
{
    // This would typically send UI notifications to players
    UE_LOG(LogTemp, Warning, TEXT("ENVIRONMENTAL HAZARD ALERT: %s (Severity: %s)"), 
           *UEnum::GetValueAsString(CurrentHazard.HazardType),
           *UEnum::GetValueAsString(CurrentHazard.Severity));

    for (const FString& Tip : CurrentHazard.SurvivalTips)
    {
        UE_LOG(LogTemp, Log, TEXT("Survival Tip: %s"), *Tip);
    }
}

void AQuest_EnvironmentalHazardSystem::InitializeResponseOptions()
{
    AvailableResponses.Empty();

    // Universal responses
    AvailableResponses.Add(CreateResponse("Build Emergency Shelter", 0.7f, 45.0f, {"Wood", "Stone"}));
    AvailableResponses.Add(CreateResponse("First Aid Treatment", 0.8f, 20.0f, {"Medicinal Plants"}));
    AvailableResponses.Add(CreateResponse("Gather Supplies", 0.9f, 30.0f, {}));

    // Evacuation responses
    AvailableResponses.Add(CreateResponse("Organize Evacuation", 0.6f, 60.0f, {}));
    AvailableResponses.Add(CreateResponse("Find Higher Ground", 0.8f, 25.0f, {}));
    AvailableResponses.Add(CreateResponse("Seek Underground Shelter", 0.5f, 40.0f, {}));

    // Fire-specific responses
    AvailableResponses.Add(CreateResponse("Create Firebreak", 0.4f, 90.0f, {"Tools"}));
    AvailableResponses.Add(CreateResponse("Find Water Source", 0.7f, 35.0f, {}));

    // Flood-specific responses
    AvailableResponses.Add(CreateResponse("Build Raft", 0.5f, 120.0f, {"Wood", "Rope"}));
    AvailableResponses.Add(CreateResponse("Secure High Ground", 0.8f, 30.0f, {}));
}

FQuest_HazardResponse AQuest_EnvironmentalHazardSystem::CreateResponse(const FString& Name, float SuccessChance, float TimeRequired, const TArray<FString>& RequiredItems)
{
    FQuest_HazardResponse Response;
    Response.ResponseName = Name;
    Response.SuccessChance = SuccessChance;
    Response.TimeRequired = TimeRequired;
    Response.RequiredItems = RequiredItems;
    Response.StaminaCost = TimeRequired * 0.5f; // Stamina cost based on time
    Response.bRequiresGroupAction = (RequiredItems.Num() > 2 || TimeRequired > 60.0f);
    
    return Response;
}