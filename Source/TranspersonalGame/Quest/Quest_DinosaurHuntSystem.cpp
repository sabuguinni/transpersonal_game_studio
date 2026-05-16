#include "Quest_DinosaurHuntSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UQuest_DinosaurHuntSystem::UQuest_DinosaurHuntSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bMissionActive = false;
    MissionStartTime = 0.0f;
}

void UQuest_DinosaurHuntSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeHuntSystem();
}

void UQuest_DinosaurHuntSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bMissionActive)
    {
        // Check mission time limit
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float ElapsedTime = CurrentTime - MissionStartTime;
        
        if (CurrentMission.TimeLimit > 0.0f && ElapsedTime >= CurrentMission.TimeLimit)
        {
            FailMission();
            return;
        }

        // Update dinosaur tracking every 5 seconds
        static float LastTrackingUpdate = 0.0f;
        if (CurrentTime - LastTrackingUpdate >= 5.0f)
        {
            UpdateDinosaurTracking();
            LastTrackingUpdate = CurrentTime;
        }

        // Check mission progress
        if (CheckMissionProgress())
        {
            CompleteMission();
        }
    }
}

void UQuest_DinosaurHuntSystem::InitializeHuntSystem()
{
    KillCounts.Empty();
    KillCounts.Add(EQuest_DinosaurSpecies::Compsognathus, 0);
    KillCounts.Add(EQuest_DinosaurSpecies::Parasaurolophus, 0);
    KillCounts.Add(EQuest_DinosaurSpecies::Triceratops, 0);
    KillCounts.Add(EQuest_DinosaurSpecies::Velociraptor, 0);
    KillCounts.Add(EQuest_DinosaurSpecies::Allosaurus, 0);
    KillCounts.Add(EQuest_DinosaurSpecies::Tyrannosaurus, 0);

    CreateBasicHuntMissions();
    LogHuntProgress(TEXT("Hunt System initialized with basic missions"));
}

void UQuest_DinosaurHuntSystem::StartHuntMission(const FString& MissionName)
{
    if (bMissionActive)
    {
        LogHuntProgress(TEXT("Cannot start new mission - mission already active"));
        return;
    }

    for (const FQuest_HuntMission& Mission : AvailableMissions)
    {
        if (Mission.MissionName == MissionName)
        {
            CurrentMission = Mission;
            bMissionActive = true;
            MissionStartTime = GetWorld()->GetTimeSeconds();
            
            // Reset kill counts for this mission
            for (auto& KillCount : KillCounts)
            {
                KillCount.Value = 0;
            }

            SpawnDinosaurTargets();
            ShowHuntObjectives();
            LogHuntProgress(FString::Printf(TEXT("Started hunt mission: %s"), *MissionName));
            return;
        }
    }

    LogHuntProgress(FString::Printf(TEXT("Mission not found: %s"), *MissionName));
}

void UQuest_DinosaurHuntSystem::CompleteMission()
{
    if (!bMissionActive)
    {
        return;
    }

    bMissionActive = false;
    DistributeRewards(CurrentMission.RewardItems);
    LogHuntProgress(FString::Printf(TEXT("Mission completed: %s"), *CurrentMission.MissionName));
    
    // Clear current mission
    CurrentMission = FQuest_HuntMission();
}

void UQuest_DinosaurHuntSystem::FailMission()
{
    if (!bMissionActive)
    {
        return;
    }

    bMissionActive = false;
    LogHuntProgress(FString::Printf(TEXT("Mission failed: %s"), *CurrentMission.MissionName));
    
    // Clear current mission
    CurrentMission = FQuest_HuntMission();
}

void UQuest_DinosaurHuntSystem::RegisterDinosaurKill(EQuest_DinosaurSpecies Species, const FVector& KillLocation)
{
    if (!bMissionActive)
    {
        return;
    }

    if (KillCounts.Contains(Species))
    {
        KillCounts[Species]++;
        LogHuntProgress(FString::Printf(TEXT("Dinosaur killed: Species %d, Total: %d"), 
            (int32)Species, KillCounts[Species]));
    }

    UpdateMissionUI();
}

bool UQuest_DinosaurHuntSystem::CheckMissionProgress()
{
    if (!bMissionActive)
    {
        return false;
    }

    for (const FQuest_DinosaurTarget& Target : CurrentMission.Targets)
    {
        int32 CurrentKills = KillCounts.Contains(Target.Species) ? KillCounts[Target.Species] : 0;
        if (CurrentKills < Target.RequiredCount)
        {
            return false;
        }
    }

    return true;
}

void UQuest_DinosaurHuntSystem::UpdateDinosaurTracking()
{
    if (!bMissionActive || !GetWorld())
    {
        return;
    }

    TrackedDinosaurs.Empty();
    
    // Find all actors in the world that could be dinosaurs
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Dinosaur")))
        {
            TrackedDinosaurs.Add(Actor);
        }
    }

    LogHuntProgress(FString::Printf(TEXT("Tracking %d dinosaurs"), TrackedDinosaurs.Num()));
}

TArray<FQuest_HuntMission> UQuest_DinosaurHuntSystem::GetAvailableMissions() const
{
    return AvailableMissions;
}

FQuest_HuntMission UQuest_DinosaurHuntSystem::GetCurrentMission() const
{
    return CurrentMission;
}

float UQuest_DinosaurHuntSystem::GetRemainingTime() const
{
    if (!bMissionActive || CurrentMission.TimeLimit <= 0.0f)
    {
        return -1.0f;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - MissionStartTime;
    return FMath::Max(0.0f, CurrentMission.TimeLimit - ElapsedTime);
}

void UQuest_DinosaurHuntSystem::CreateBasicHuntMissions()
{
    AvailableMissions.Empty();
    
    CreateScavengerMissions();
    CreateHunterMissions();
    CreatePredatorMissions();
    CreateApexMissions();
}

void UQuest_DinosaurHuntSystem::SpawnDinosaurTargets()
{
    if (!GetWorld())
    {
        return;
    }

    // Spawn placeholder dinosaur actors for mission targets
    for (const FQuest_DinosaurTarget& Target : CurrentMission.Targets)
    {
        FVector SpawnLocation = CurrentMission.StartLocation + FVector(
            FMath::RandRange(-CurrentMission.SearchRadius, CurrentMission.SearchRadius),
            FMath::RandRange(-CurrentMission.SearchRadius, CurrentMission.SearchRadius),
            100.0f
        );

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AActor* DinosaurActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (DinosaurActor)
        {
            FString SpeciesName;
            switch (Target.Species)
            {
                case EQuest_DinosaurSpecies::Compsognathus: SpeciesName = TEXT("Compsognathus"); break;
                case EQuest_DinosaurSpecies::Parasaurolophus: SpeciesName = TEXT("Parasaurolophus"); break;
                case EQuest_DinosaurSpecies::Triceratops: SpeciesName = TEXT("Triceratops"); break;
                case EQuest_DinosaurSpecies::Velociraptor: SpeciesName = TEXT("Velociraptor"); break;
                case EQuest_DinosaurSpecies::Allosaurus: SpeciesName = TEXT("Allosaurus"); break;
                case EQuest_DinosaurSpecies::Tyrannosaurus: SpeciesName = TEXT("Tyrannosaurus"); break;
            }
            
            DinosaurActor->SetActorLabel(FString::Printf(TEXT("Dinosaur_%s_Hunt"), *SpeciesName));
            TrackedDinosaurs.Add(DinosaurActor);
        }
    }
}

AActor* UQuest_DinosaurHuntSystem::FindNearestDinosaur(EQuest_DinosaurSpecies Species, const FVector& SearchLocation)
{
    AActor* NearestDinosaur = nullptr;
    float NearestDistance = FLT_MAX;

    FString SpeciesName;
    switch (Species)
    {
        case EQuest_DinosaurSpecies::Compsognathus: SpeciesName = TEXT("Compsognathus"); break;
        case EQuest_DinosaurSpecies::Parasaurolophus: SpeciesName = TEXT("Parasaurolophus"); break;
        case EQuest_DinosaurSpecies::Triceratops: SpeciesName = TEXT("Triceratops"); break;
        case EQuest_DinosaurSpecies::Velociraptor: SpeciesName = TEXT("Velociraptor"); break;
        case EQuest_DinosaurSpecies::Allosaurus: SpeciesName = TEXT("Allosaurus"); break;
        case EQuest_DinosaurSpecies::Tyrannosaurus: SpeciesName = TEXT("Tyrannosaurus"); break;
    }

    for (AActor* Dinosaur : TrackedDinosaurs)
    {
        if (Dinosaur && Dinosaur->GetActorLabel().Contains(SpeciesName))
        {
            float Distance = FVector::Dist(SearchLocation, Dinosaur->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestDinosaur = Dinosaur;
            }
        }
    }

    return NearestDinosaur;
}

void UQuest_DinosaurHuntSystem::SetHuntWaypoint(const FVector& Location)
{
    LogHuntProgress(FString::Printf(TEXT("Hunt waypoint set at: %s"), *Location.ToString()));
}

void UQuest_DinosaurHuntSystem::ShowHuntObjectives()
{
    if (!bMissionActive)
    {
        return;
    }

    LogHuntProgress(FString::Printf(TEXT("=== HUNT MISSION: %s ==="), *CurrentMission.MissionName));
    LogHuntProgress(CurrentMission.MissionDescription);
    
    for (int32 i = 0; i < CurrentMission.Targets.Num(); i++)
    {
        const FQuest_DinosaurTarget& Target = CurrentMission.Targets[i];
        int32 CurrentKills = KillCounts.Contains(Target.Species) ? KillCounts[Target.Species] : 0;
        
        FString SpeciesName;
        switch (Target.Species)
        {
            case EQuest_DinosaurSpecies::Compsognathus: SpeciesName = TEXT("Compsognathus"); break;
            case EQuest_DinosaurSpecies::Parasaurolophus: SpeciesName = TEXT("Parasaurolophus"); break;
            case EQuest_DinosaurSpecies::Triceratops: SpeciesName = TEXT("Triceratops"); break;
            case EQuest_DinosaurSpecies::Velociraptor: SpeciesName = TEXT("Velociraptor"); break;
            case EQuest_DinosaurSpecies::Allosaurus: SpeciesName = TEXT("Allosaurus"); break;
            case EQuest_DinosaurSpecies::Tyrannosaurus: SpeciesName = TEXT("Tyrannosaurus"); break;
        }
        
        LogHuntProgress(FString::Printf(TEXT("Target %d: %s (%d/%d)"), 
            i + 1, *SpeciesName, CurrentKills, Target.RequiredCount));
    }
    
    if (CurrentMission.TimeLimit > 0.0f)
    {
        LogHuntProgress(FString::Printf(TEXT("Time Limit: %.0f minutes"), CurrentMission.TimeLimit / 60.0f));
    }
}

void UQuest_DinosaurHuntSystem::CreateScavengerMissions()
{
    FQuest_HuntMission ScavengerMission;
    ScavengerMission.MissionName = TEXT("First Blood");
    ScavengerMission.MissionDescription = TEXT("Hunt small scavengers to learn basic tracking and killing techniques. Start with the smallest prey.");
    ScavengerMission.TimeLimit = 1200.0f; // 20 minutes
    ScavengerMission.SearchRadius = 5000.0f;
    ScavengerMission.bRequiresGroupHunt = false;
    ScavengerMission.MinimumHunters = 1;
    
    FQuest_DinosaurTarget CompyTarget;
    CompyTarget.Species = EQuest_DinosaurSpecies::Compsognathus;
    CompyTarget.RequiredCount = 3;
    CompyTarget.Difficulty = EQuest_HuntDifficulty::Scavenger;
    CompyTarget.TrackingRadius = 2000.0f;
    CompyTarget.bRequiresSpecificWeapon = false;
    
    ScavengerMission.Targets.Add(CompyTarget);
    ScavengerMission.RewardItems.Add(TEXT("Small Hide"));
    ScavengerMission.RewardItems.Add(TEXT("Raw Meat"));
    
    AvailableMissions.Add(ScavengerMission);
}

void UQuest_DinosaurHuntSystem::CreateHunterMissions()
{
    FQuest_HuntMission HunterMission;
    HunterMission.MissionName = TEXT("Herbivore Hunter");
    HunterMission.MissionDescription = TEXT("Track and hunt medium-sized herbivores. These creatures are peaceful but provide substantial resources.");
    HunterMission.TimeLimit = 2400.0f; // 40 minutes
    HunterMission.SearchRadius = 8000.0f;
    HunterMission.bRequiresGroupHunt = false;
    HunterMission.MinimumHunters = 1;
    
    FQuest_DinosaurTarget ParaTarget;
    ParaTarget.Species = EQuest_DinosaurSpecies::Parasaurolophus;
    ParaTarget.RequiredCount = 1;
    ParaTarget.Difficulty = EQuest_HuntDifficulty::Hunter;
    ParaTarget.TrackingRadius = 3000.0f;
    ParaTarget.bRequiresSpecificWeapon = true;
    ParaTarget.RequiredWeaponType = TEXT("Stone Spear");
    
    HunterMission.Targets.Add(ParaTarget);
    HunterMission.RewardItems.Add(TEXT("Large Hide"));
    HunterMission.RewardItems.Add(TEXT("Prime Meat"));
    HunterMission.RewardItems.Add(TEXT("Bone Tools"));
    
    AvailableMissions.Add(HunterMission);
}

void UQuest_DinosaurHuntSystem::CreatePredatorMissions()
{
    FQuest_HuntMission PredatorMission;
    PredatorMission.MissionName = TEXT("Pack Hunter");
    PredatorMission.MissionDescription = TEXT("Hunt dangerous pack predators. This requires strategy, skill, and courage. Recommended for experienced hunters only.");
    PredatorMission.TimeLimit = 3600.0f; // 60 minutes
    PredatorMission.SearchRadius = 12000.0f;
    PredatorMission.bRequiresGroupHunt = true;
    PredatorMission.MinimumHunters = 2;
    
    FQuest_DinosaurTarget RaptorTarget;
    RaptorTarget.Species = EQuest_DinosaurSpecies::Velociraptor;
    RaptorTarget.RequiredCount = 2;
    RaptorTarget.Difficulty = EQuest_HuntDifficulty::Predator;
    RaptorTarget.TrackingRadius = 5000.0f;
    RaptorTarget.bRequiresSpecificWeapon = true;
    RaptorTarget.RequiredWeaponType = TEXT("Obsidian Spear");
    
    PredatorMission.Targets.Add(RaptorTarget);
    PredatorMission.RewardItems.Add(TEXT("Raptor Claw"));
    PredatorMission.RewardItems.Add(TEXT("Predator Hide"));
    PredatorMission.RewardItems.Add(TEXT("Sharp Teeth"));
    
    AvailableMissions.Add(PredatorMission);
}

void UQuest_DinosaurHuntSystem::CreateApexMissions()
{
    FQuest_HuntMission ApexMission;
    ApexMission.MissionName = TEXT("Apex Challenge");
    ApexMission.MissionDescription = TEXT("Face the ultimate predator. Only the most skilled and prepared hunters should attempt this legendary challenge.");
    ApexMission.TimeLimit = 7200.0f; // 120 minutes
    ApexMission.SearchRadius = 20000.0f;
    ApexMission.bRequiresGroupHunt = true;
    ApexMission.MinimumHunters = 4;
    
    FQuest_DinosaurTarget TRexTarget;
    TRexTarget.Species = EQuest_DinosaurSpecies::Tyrannosaurus;
    TRexTarget.RequiredCount = 1;
    TRexTarget.Difficulty = EQuest_HuntDifficulty::Apex;
    TRexTarget.TrackingRadius = 10000.0f;
    TRexTarget.bRequiresSpecificWeapon = true;
    TRexTarget.RequiredWeaponType = TEXT("Hardened Spear");
    
    ApexMission.Targets.Add(TRexTarget);
    ApexMission.RewardItems.Add(TEXT("T-Rex Tooth"));
    ApexMission.RewardItems.Add(TEXT("Apex Hide"));
    ApexMission.RewardItems.Add(TEXT("Legendary Bone"));
    ApexMission.RewardItems.Add(TEXT("Alpha Trophy"));
    
    AvailableMissions.Add(ApexMission);
}

bool UQuest_DinosaurHuntSystem::ValidateWeaponRequirement(const FString& WeaponType)
{
    // This would integrate with the inventory/weapon system
    LogHuntProgress(FString::Printf(TEXT("Validating weapon requirement: %s"), *WeaponType));
    return true; // Placeholder - assume player has required weapon
}

void UQuest_DinosaurHuntSystem::DistributeRewards(const TArray<FString>& Rewards)
{
    for (const FString& Reward : Rewards)
    {
        LogHuntProgress(FString::Printf(TEXT("Reward received: %s"), *Reward));
        // This would integrate with the inventory system to actually give items
    }
}

void UQuest_DinosaurHuntSystem::UpdateMissionUI()
{
    // This would update the UI to show current progress
    LogHuntProgress(TEXT("Mission UI updated"));
}

void UQuest_DinosaurHuntSystem::LogHuntProgress(const FString& Message)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, 
            FString::Printf(TEXT("[Hunt System] %s"), *Message));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[Quest_DinosaurHuntSystem] %s"), *Message);
}