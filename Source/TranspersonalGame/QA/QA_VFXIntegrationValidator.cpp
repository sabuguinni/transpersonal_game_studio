#include "QA_VFXIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystem.h"
#include "Engine/StaticMeshActor.h"
#include "../VFXSystem/VFX_FootstepManager.h"

AQA_VFXIntegrationValidator::AQA_VFXIntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize default test configuration
    MaxAllowedFrameTime = 16.67f; // 60 FPS target
    MaxParticleSystemsAllowed = 50;
    VFXTestRadius = 2000.0f;
    LastPerformanceScore = 0.0f;
    bAllTestsPassed = false;
    
    // Initialize dinosaur VFX profiles
    InitializeDinosaurVFXProfiles();
}

void AQA_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Starting VFX validation system"));
    
    // Run initial validation suite
    TArray<FQA_VFXTestResult> InitialResults = RunFullVFXValidationSuite();
    
    // Log results
    for (const FQA_VFXTestResult& Result : InitialResults)
    {
        if (Result.bTestPassed)
        {
            UE_LOG(LogTemp, Warning, TEXT("✓ VFX Test PASSED: %s (Score: %.2f)"), *Result.TestName, Result.PerformanceScore);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("✗ VFX Test FAILED: %s - %s"), *Result.TestName, *Result.ErrorMessage);
        }
    }
}

void AQA_VFXIntegrationValidator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Continuous performance monitoring
    static float MonitoringTimer = 0.0f;
    MonitoringTimer += DeltaTime;
    
    if (MonitoringTimer >= 5.0f) // Check every 5 seconds
    {
        LastPerformanceScore = GetCurrentVFXPerformanceScore();
        bool bWithinBudget = IsVFXSystemWithinPerformanceBudget();
        
        if (!bWithinBudget)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA WARNING: VFX system exceeding performance budget! Score: %.2f"), LastPerformanceScore);
        }
        
        MonitoringTimer = 0.0f;
    }
}

FQA_VFXTestResult AQA_VFXIntegrationValidator::ValidateFootstepVFXSystem()
{
    FQA_VFXTestResult Result = CreateTestResult(TEXT("Footstep VFX System"), false);
    
    try
    {
        // Find VFX_FootstepManager actors in the world
        TArray<AActor*> FootstepManagers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVFX_FootstepManager::StaticClass(), FootstepManagers);
        
        if (FootstepManagers.Num() == 0)
        {
            Result.ErrorMessage = TEXT("No VFX_FootstepManager actors found in world");
            return Result;
        }
        
        // Test each footstep manager
        int32 ValidManagers = 0;
        int32 TotalParticles = 0;
        
        for (AActor* Manager : FootstepManagers)
        {
            AVFX_FootstepManager* FootstepMgr = Cast<AVFX_FootstepManager>(Manager);
            if (FootstepMgr)
            {
                // Test particle system components
                TArray<UParticleSystemComponent*> ParticleComps;
                FootstepMgr->GetComponents<UParticleSystemComponent>(ParticleComps);
                
                for (UParticleSystemComponent* ParticleComp : ParticleComps)
                {
                    if (ValidateParticleSystemComponent(ParticleComp))
                    {
                        TotalParticles++;
                    }
                }
                
                ValidManagers++;
            }
        }
        
        if (ValidManagers > 0)
        {
            Result.bTestPassed = true;
            Result.ParticleCount = TotalParticles;
            Result.PerformanceScore = FMath::Clamp(100.0f - (TotalParticles * 2.0f), 0.0f, 100.0f);
        }
        else
        {
            Result.ErrorMessage = TEXT("No valid VFX_FootstepManager components found");
        }
    }
    catch (...)
    {
        Result.ErrorMessage = TEXT("Exception during footstep VFX validation");
    }
    
    return Result;
}

FQA_VFXTestResult AQA_VFXIntegrationValidator::ValidateCampfireVFXSystem()
{
    FQA_VFXTestResult Result = CreateTestResult(TEXT("Campfire VFX System"), false);
    
    try
    {
        // Find campfire-related actors
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        int32 CampfireActors = 0;
        int32 CampfireParticles = 0;
        int32 CampfireLights = 0;
        
        for (AActor* Actor : AllActors)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Campfire")) || ActorName.Contains(TEXT("Fire")))
            {
                CampfireActors++;
                
                // Check for particle systems
                TArray<UParticleSystemComponent*> ParticleComps;
                Actor->GetComponents<UParticleSystemComponent>(ParticleComps);
                CampfireParticles += ParticleComps.Num();
                
                // Check for light components
                TArray<ULightComponent*> LightComps;
                Actor->GetComponents<ULightComponent>(LightComps);
                CampfireLights += LightComps.Num();
            }
        }
        
        if (CampfireActors > 0 && (CampfireParticles > 0 || CampfireLights > 0))
        {
            Result.bTestPassed = true;
            Result.ParticleCount = CampfireParticles;
            Result.PerformanceScore = FMath::Clamp(90.0f - (CampfireParticles * 1.5f), 0.0f, 100.0f);
        }
        else
        {
            Result.ErrorMessage = FString::Printf(TEXT("Insufficient campfire VFX: %d actors, %d particles, %d lights"), 
                CampfireActors, CampfireParticles, CampfireLights);
        }
    }
    catch (...)
    {
        Result.ErrorMessage = TEXT("Exception during campfire VFX validation");
    }
    
    return Result;
}

FQA_VFXTestResult AQA_VFXIntegrationValidator::ValidateEnvironmentalVFX()
{
    FQA_VFXTestResult Result = CreateTestResult(TEXT("Environmental VFX"), false);
    
    try
    {
        // Count all particle systems in the world
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        int32 TotalParticleSystems = 0;
        int32 ActiveParticleSystems = 0;
        
        for (AActor* Actor : AllActors)
        {
            TArray<UParticleSystemComponent*> ParticleComps;
            Actor->GetComponents<UParticleSystemComponent>(ParticleComps);
            
            for (UParticleSystemComponent* ParticleComp : ParticleComps)
            {
                TotalParticleSystems++;
                if (ParticleComp && ParticleComp->IsActive())
                {
                    ActiveParticleSystems++;
                }
            }
        }
        
        // Check atmospheric fog
        bool bHasFog = false;
        TArray<AActor*> FogActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAtmosphericFog::StaticClass(), FogActors);
        bHasFog = FogActors.Num() > 0;
        
        if (TotalParticleSystems >= 3 && ActiveParticleSystems >= 2)
        {
            Result.bTestPassed = true;
            Result.ParticleCount = ActiveParticleSystems;
            Result.PerformanceScore = FMath::Clamp(85.0f - (ActiveParticleSystems * 1.0f), 0.0f, 100.0f);
            
            if (bHasFog)
            {
                Result.PerformanceScore += 10.0f; // Bonus for atmospheric effects
            }
        }
        else
        {
            Result.ErrorMessage = FString::Printf(TEXT("Insufficient environmental VFX: %d total, %d active particle systems"), 
                TotalParticleSystems, ActiveParticleSystems);
        }
    }
    catch (...)
    {
        Result.ErrorMessage = TEXT("Exception during environmental VFX validation");
    }
    
    return Result;
}

FQA_VFXTestResult AQA_VFXIntegrationValidator::ValidateVFXPerformance()
{
    FQA_VFXTestResult Result = CreateTestResult(TEXT("VFX Performance"), false);
    
    try
    {
        float CurrentPerformanceScore = GetCurrentVFXPerformanceScore();
        int32 ActiveParticleCount = GetActiveParticleSystemCount();
        bool bWithinBudget = IsVFXSystemWithinPerformanceBudget();
        
        Result.PerformanceScore = CurrentPerformanceScore;
        Result.ParticleCount = ActiveParticleCount;
        
        if (bWithinBudget && CurrentPerformanceScore >= 60.0f)
        {
            Result.bTestPassed = true;
        }
        else
        {
            Result.ErrorMessage = FString::Printf(TEXT("Performance below threshold: Score %.2f, %d particles, Budget OK: %s"), 
                CurrentPerformanceScore, ActiveParticleCount, bWithinBudget ? TEXT("Yes") : TEXT("No"));
        }
    }
    catch (...)
    {
        Result.ErrorMessage = TEXT("Exception during VFX performance validation");
    }
    
    return Result;
}

TArray<FQA_VFXTestResult> AQA_VFXIntegrationValidator::RunFullVFXValidationSuite()
{
    TArray<FQA_VFXTestResult> Results;
    
    // Run all validation tests
    Results.Add(ValidateFootstepVFXSystem());
    Results.Add(ValidateCampfireVFXSystem());
    Results.Add(ValidateEnvironmentalVFX());
    Results.Add(ValidateVFXPerformance());
    
    // Test dinosaur-specific VFX
    Results.Add(ValidateDinosaurVFXProfile(EDinosaurSpecies::TRex));
    Results.Add(ValidateDinosaurVFXProfile(EDinosaurSpecies::Raptor));
    Results.Add(ValidateDinosaurVFXProfile(EDinosaurSpecies::Brachiosaurus));
    
    // Store results
    LastTestResults = Results;
    
    // Calculate overall pass rate
    int32 PassedTests = 0;
    for (const FQA_VFXTestResult& Result : Results)
    {
        if (Result.bTestPassed)
        {
            PassedTests++;
        }
    }
    
    bAllTestsPassed = (PassedTests == Results.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("QA VFX Validation Complete: %d/%d tests passed"), PassedTests, Results.Num());
    
    return Results;
}

FQA_VFXTestResult AQA_VFXIntegrationValidator::ValidateDinosaurVFXProfile(EDinosaurSpecies Species)
{
    FString TestName = FString::Printf(TEXT("Dinosaur VFX Profile - %s"), 
        *UEnum::GetValueAsString(Species));
    FQA_VFXTestResult Result = CreateTestResult(TestName, false);
    
    // Find matching profile
    FQA_DinosaurVFXProfile* Profile = nullptr;
    for (FQA_DinosaurVFXProfile& TestProfile : DinosaurVFXProfiles)
    {
        if (TestProfile.Species == Species)
        {
            Profile = &TestProfile;
            break;
        }
    }
    
    if (!Profile)
    {
        Result.ErrorMessage = TEXT("No VFX profile found for species");
        return Result;
    }
    
    // Validate profile parameters
    bool bValidProfile = true;
    if (Profile->ExpectedDustIntensity <= 0.0f || Profile->ExpectedDustIntensity > 5.0f)
    {
        bValidProfile = false;
        Result.ErrorMessage += TEXT("Invalid dust intensity; ");
    }
    
    if (Profile->ExpectedParticleSize <= 0.0f || Profile->ExpectedParticleSize > 500.0f)
    {
        bValidProfile = false;
        Result.ErrorMessage += TEXT("Invalid particle size; ");
    }
    
    if (Profile->MassKg <= 0.0f || Profile->MassKg > 100000.0f)
    {
        bValidProfile = false;
        Result.ErrorMessage += TEXT("Invalid mass; ");
    }
    
    if (bValidProfile)
    {
        Result.bTestPassed = true;
        Result.PerformanceScore = 95.0f;
    }
    
    return Result;
}

bool AQA_VFXIntegrationValidator::TestFootstepVFXForAllSpecies()
{
    bool bAllSpeciesValid = true;
    
    for (const FQA_DinosaurVFXProfile& Profile : DinosaurVFXProfiles)
    {
        FQA_VFXTestResult Result = ValidateDinosaurVFXProfile(Profile.Species);
        if (!Result.bTestPassed)
        {
            bAllSpeciesValid = false;
            UE_LOG(LogTemp, Error, TEXT("Species %s failed VFX validation: %s"), 
                *UEnum::GetValueAsString(Profile.Species), *Result.ErrorMessage);
        }
    }
    
    return bAllSpeciesValid;
}

float AQA_VFXIntegrationValidator::GetCurrentVFXPerformanceScore()
{
    int32 ActiveParticles = GetActiveParticleSystemCount();
    
    // Base score starts at 100, decreases with particle count
    float Score = 100.0f - (ActiveParticles * 1.5f);
    
    // Bonus for having some VFX (not completely empty)
    if (ActiveParticles > 0 && ActiveParticles <= 10)
    {
        Score += 10.0f;
    }
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

int32 AQA_VFXIntegrationValidator::GetActiveParticleSystemCount()
{
    int32 Count = 0;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        TArray<UParticleSystemComponent*> ParticleComps;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComps);
        
        for (UParticleSystemComponent* ParticleComp : ParticleComps)
        {
            if (ParticleComp && ParticleComp->IsActive())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

bool AQA_VFXIntegrationValidator::IsVFXSystemWithinPerformanceBudget()
{
    int32 ActiveParticles = GetActiveParticleSystemCount();
    return ActiveParticles <= MaxParticleSystemsAllowed;
}

bool AQA_VFXIntegrationValidator::ValidateParticleSystemComponent(UParticleSystemComponent* ParticleComp)
{
    if (!ParticleComp)
    {
        return false;
    }
    
    // Check if component has a valid particle system
    if (!ParticleComp->GetTemplate())
    {
        return false;
    }
    
    // Check if component is properly initialized
    if (!ParticleComp->IsRegistered())
    {
        return false;
    }
    
    return true;
}

float AQA_VFXIntegrationValidator::CalculateVFXPerformanceImpact(AActor* VFXActor)
{
    if (!VFXActor)
    {
        return 0.0f;
    }
    
    float Impact = 0.0f;
    
    // Count particle systems
    TArray<UParticleSystemComponent*> ParticleComps;
    VFXActor->GetComponents<UParticleSystemComponent>(ParticleComps);
    Impact += ParticleComps.Num() * 2.0f;
    
    // Count light components
    TArray<ULightComponent*> LightComps;
    VFXActor->GetComponents<ULightComponent>(LightComps);
    Impact += LightComps.Num() * 1.5f;
    
    // Count audio components
    TArray<UAudioComponent*> AudioComps;
    VFXActor->GetComponents<UAudioComponent>(AudioComps);
    Impact += AudioComps.Num() * 1.0f;
    
    return Impact;
}

bool AQA_VFXIntegrationValidator::CheckVFXAudioSynchronization(AActor* VFXActor)
{
    if (!VFXActor)
    {
        return false;
    }
    
    // Check if VFX actor has both particle and audio components
    TArray<UParticleSystemComponent*> ParticleComps;
    VFXActor->GetComponents<UParticleSystemComponent>(ParticleComps);
    
    TArray<UAudioComponent*> AudioComps;
    VFXActor->GetComponents<UAudioComponent>(AudioComps);
    
    // If has particles, should have audio for immersion
    if (ParticleComps.Num() > 0 && AudioComps.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Actor %s has particles but no audio"), *VFXActor->GetName());
        return false;
    }
    
    return true;
}

void AQA_VFXIntegrationValidator::InitializeDinosaurVFXProfiles()
{
    DinosaurVFXProfiles.Empty();
    
    // T-Rex profile
    FQA_DinosaurVFXProfile TRexProfile;
    TRexProfile.Species = EDinosaurSpecies::TRex;
    TRexProfile.ExpectedDustIntensity = 2.0f;
    TRexProfile.ExpectedParticleSize = 200.0f;
    TRexProfile.MassKg = 12000.0f;
    TRexProfile.bShouldCreateGroundCracks = true;
    DinosaurVFXProfiles.Add(TRexProfile);
    
    // Raptor profile
    FQA_DinosaurVFXProfile RaptorProfile;
    RaptorProfile.Species = EDinosaurSpecies::Raptor;
    RaptorProfile.ExpectedDustIntensity = 0.8f;
    RaptorProfile.ExpectedParticleSize = 80.0f;
    RaptorProfile.MassKg = 150.0f;
    RaptorProfile.bShouldCreateGroundCracks = false;
    DinosaurVFXProfiles.Add(RaptorProfile);
    
    // Brachiosaurus profile
    FQA_DinosaurVFXProfile BrachiosaurusProfile;
    BrachiosaurusProfile.Species = EDinosaurSpecies::Brachiosaurus;
    BrachiosaurusProfile.ExpectedDustIntensity = 3.0f;
    BrachiosaurusProfile.ExpectedParticleSize = 250.0f;
    BrachiosaurusProfile.MassKg = 80000.0f;
    BrachiosaurusProfile.bShouldCreateGroundCracks = true;
    DinosaurVFXProfiles.Add(BrachiosaurusProfile);
}

FQA_VFXTestResult AQA_VFXIntegrationValidator::CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMsg)
{
    FQA_VFXTestResult Result;
    Result.TestName = TestName;
    Result.bTestPassed = bPassed;
    Result.ErrorMessage = ErrorMsg;
    Result.PerformanceScore = bPassed ? 100.0f : 0.0f;
    Result.ParticleCount = 0;
    return Result;
}