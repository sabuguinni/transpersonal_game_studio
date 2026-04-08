#include "PhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Core/RealitySystem.h"

UPhysicsManager::UPhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics; // Tick before physics simulation
    
    // Initialize default values
    BaseGravity = -980.0f;
    MinGravityMultiplier = 0.1f;
    MaxGravityMultiplier = 2.0f;
    ConsciousnessTimeDilation = 1.0f;
    ConsciousnessAirResistance = 1.0f;
    bTranspersonalPhysicsEnabled = true;
    CurrentGravityMultiplier = 1.0f;
    LastFrameTime = 0.0f;
    AverageFrameTime = 0.0f;
}

void UPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache system references for performance
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            ConsciousnessSystem = GameInstance->GetSubsystem<UConsciousnessSystem>();
            RealitySystem = GameInstance->GetSubsystem<URealitySystem>();
        }
    }
    
    // Validate initial physics settings
    if (!ConsciousnessSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: ConsciousnessSystem not found"));
    }
    
    if (!RealitySystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: RealitySystem not found"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Initialized with base gravity %.2f"), BaseGravity);
}

void UPhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance tracking start
    double StartTime = FPlatformTime::Seconds();
    
    if (!bTranspersonalPhysicsEnabled)
    {
        return;
    }
    
    // Get current consciousness and reality state
    float ConsciousnessLevel = 0.5f; // Default fallback
    float RealityStability = 1.0f;   // Default fallback
    
    if (ConsciousnessSystem)
    {
        ConsciousnessLevel = ConsciousnessSystem->GetCurrentConsciousnessLevel();
    }
    
    if (RealitySystem)
    {
        RealityStability = RealitySystem->GetRealityStability();
    }
    
    // Update physics parameters
    UpdatePhysicsFromConsciousness(ConsciousnessLevel, RealityStability);
    
    // Process active reality distortions
    ProcessRealityDistortions(DeltaTime);
    
    // Performance tracking end
    double EndTime = FPlatformTime::Seconds();
    LastFrameTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    ValidatePerformance(DeltaTime);
}

void UPhysicsManager::UpdatePhysicsFromConsciousness(float ConsciousnessLevel, float RealityStability)
{
    // Clamp input values
    ConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.0f, 1.0f);
    RealityStability = FMath::Clamp(RealityStability, 0.0f, 1.0f);
    
    // Update gravity based on consciousness
    UpdateGravity(ConsciousnessLevel);
    
    // Update time dilation based on reality stability
    UpdateTimeDilation(RealityStability);
    
    // Update air resistance for floating/levitation effects
    float ConsciousnessInfluence = FMath::Lerp(1.0f, 0.3f, ConsciousnessLevel);
    ConsciousnessAirResistance = ConsciousnessInfluence;
}

void UPhysicsManager::ApplyConsciousnessForce(AActor* Actor, float ConsciousnessIntensity)
{
    if (!Actor || !bTranspersonalPhysicsEnabled)
    {
        return;
    }
    
    // Clamp intensity
    ConsciousnessIntensity = FMath::Clamp(ConsciousnessIntensity, 0.0f, 2.0f);
    
    // Get the actor's primitive component
    UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimitiveComp || !PrimitiveComp->IsSimulatingPhysics())
    {
        return;
    }
    
    // Calculate consciousness-influenced force
    FVector UpwardForce = FVector(0.0f, 0.0f, 1.0f);
    float ForceMultiplier = ConsciousnessIntensity * 100000.0f; // Scale for UE5 physics
    
    // Apply anti-gravity force based on consciousness intensity
    FVector FinalForce = UpwardForce * ForceMultiplier * PrimitiveComp->GetMass();
    
    // Apply the force
    PrimitiveComp->AddForce(FinalForce, NAME_None, false);
    
    // Add some random variation for organic feel
    FVector RandomForce = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ) * ForceMultiplier * 0.1f;
    
    PrimitiveComp->AddForce(RandomForce, NAME_None, false);
}

void UPhysicsManager::CreateRealityDistortion(FVector Location, float Radius, float Intensity, float Duration)
{
    if (!bTranspersonalPhysicsEnabled)
    {
        return;
    }
    
    // Create new distortion
    FRealityDistortion NewDistortion;
    NewDistortion.Location = Location;
    NewDistortion.Radius = FMath::Clamp(Radius, 100.0f, 5000.0f);
    NewDistortion.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    NewDistortion.RemainingDuration = FMath::Clamp(Duration, 0.1f, 60.0f);
    NewDistortion.DistortionType = FMath::RandRange(0, 2); // Random distortion type
    
    // Add to active distortions
    ActiveDistortions.Add(NewDistortion);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Created reality distortion at %s, radius %.2f, intensity %.2f"), 
           *Location.ToString(), Radius, Intensity);
}

float UPhysicsManager::GetConsciousnessGravityMultiplier() const
{
    return CurrentGravityMultiplier;
}

void UPhysicsManager::SetTranspersonalPhysicsEnabled(bool bEnabled)
{
    bTranspersonalPhysicsEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Reset to default physics when disabled
        if (UWorld* World = GetWorld())
        {
            if (AWorldSettings* WorldSettings = World->GetWorldSettings())
            {
                WorldSettings->GlobalGravityZ = BaseGravity;
            }
        }
        
        // Clear active distortions
        ActiveDistortions.Empty();
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Transpersonal physics %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPhysicsManager::UpdateGravity(float ConsciousnessLevel)
{
    // Higher consciousness = lower gravity (more ethereal)
    // Lower consciousness = higher gravity (more grounded)
    float TargetMultiplier = FMath::Lerp(MaxGravityMultiplier, MinGravityMultiplier, ConsciousnessLevel);
    
    // Smooth interpolation to avoid jarring changes
    CurrentGravityMultiplier = FMath::FInterpTo(CurrentGravityMultiplier, TargetMultiplier, 
                                                GetWorld()->GetDeltaSeconds(), 2.0f);
    
    // Apply to world settings
    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            float NewGravity = BaseGravity * CurrentGravityMultiplier;
            WorldSettings->GlobalGravityZ = NewGravity;
        }
    }
}

void UPhysicsManager::UpdateTimeDilation(float RealityStability)
{
    // Lower reality stability = slower time (reality breaking down)
    float TargetDilation = FMath::Lerp(0.5f, 1.0f, RealityStability);
    
    // Smooth interpolation
    ConsciousnessTimeDilation = FMath::FInterpTo(ConsciousnessTimeDilation, TargetDilation,
                                                 GetWorld()->GetDeltaSeconds(), 1.0f);
    
    // Apply to world settings
    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            WorldSettings->SetTimeDilation(ConsciousnessTimeDilation);
        }
    }
}

void UPhysicsManager::ProcessRealityDistortions(float DeltaTime)
{
    // Process each active distortion
    for (int32 i = ActiveDistortions.Num() - 1; i >= 0; i--)
    {
        FRealityDistortion& Distortion = ActiveDistortions[i];
        
        // Update duration
        Distortion.RemainingDuration -= DeltaTime;
        
        // Remove expired distortions
        if (Distortion.RemainingDuration <= 0.0f)
        {
            ActiveDistortions.RemoveAt(i);
            continue;
        }
        
        // Apply distortion effects to nearby actors
        if (UWorld* World = GetWorld())
        {
            TArray<AActor*> NearbyActors;
            
            // Find actors within distortion radius
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (!Actor) continue;
                
                float Distance = FVector::Dist(Actor->GetActorLocation(), Distortion.Location);
                if (Distance <= Distortion.Radius)
                {
                    // Apply distortion-specific effects
                    float DistanceRatio = 1.0f - (Distance / Distortion.Radius);
                    float EffectIntensity = Distortion.Intensity * DistanceRatio;
                    
                    // Apply consciousness force based on distortion type
                    ApplyConsciousnessForce(Actor, EffectIntensity);
                }
            }
        }
    }
}

void UPhysicsManager::ValidatePerformance(float DeltaTime)
{
    // Update running average
    AverageFrameTime = (AverageFrameTime * 0.9f) + (LastFrameTime * 0.1f);
    
    // Check if we're exceeding our performance budget
    const float MaxBudgetMs = 1.2f; // Maximum allowed frame time
    const float TargetBudgetMs = 0.8f; // Target frame time
    
    if (AverageFrameTime > MaxBudgetMs)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Performance budget exceeded! Average: %.3fms (Max: %.3fms)"), 
               AverageFrameTime, MaxBudgetMs);
        
        // Emergency performance measures
        if (ActiveDistortions.Num() > 10)
        {
            // Remove oldest distortions
            ActiveDistortions.RemoveAt(0, ActiveDistortions.Num() / 2);
            UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Reduced active distortions for performance"));
        }
    }
    else if (AverageFrameTime <= TargetBudgetMs)
    {
        // Performance is good - could potentially add more features
        static int32 GoodFrameCounter = 0;
        GoodFrameCounter++;
        
        if (GoodFrameCounter % 300 == 0) // Log every 5 seconds at 60fps
        {
            UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Performance excellent - %.3fms average"), AverageFrameTime);
        }
    }
}