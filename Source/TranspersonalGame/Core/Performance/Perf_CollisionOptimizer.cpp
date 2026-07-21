#include "Perf_CollisionOptimizer.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UPerf_CollisionOptimizer::UPerf_CollisionOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10fps for optimization checks

    // Initialize performance profiles
    HighQualityProfile.MaxDistance = 8000.0f;
    HighQualityProfile.MaxCollisionChecks = 200;
    HighQualityProfile.UpdateFrequency = 0.016f;
    HighQualityProfile.bEnableComplexCollision = true;

    MediumQualityProfile.MaxDistance = 5000.0f;
    MediumQualityProfile.MaxCollisionChecks = 100;
    MediumQualityProfile.UpdateFrequency = 0.033f;
    MediumQualityProfile.bEnableComplexCollision = true;

    LowQualityProfile.MaxDistance = 3000.0f;
    LowQualityProfile.MaxCollisionChecks = 50;
    LowQualityProfile.UpdateFrequency = 0.066f;
    LowQualityProfile.bEnableComplexCollision = false;
}

void UPerf_CollisionOptimizer::BeginPlay()
{
    Super::BeginPlay();

    // Find all primitive components in the world to track
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* Component : PrimitiveComponents)
                {
                    if (Component && Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                    {
                        TrackedComponents.Add(Component);
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CollisionOptimizer: Tracking %d collision components"), TrackedComponents.Num());
}

void UPerf_CollisionOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastPerformanceCheck += DeltaTime;
    
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bEnableDynamicLOD)
        {
            CullDistantCollisions();
            UpdateCollisionComplexity();
        }
        
        LastPerformanceCheck = 0.0f;
    }
}

void UPerf_CollisionOptimizer::OptimizeCollisionForDistance(float Distance)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    CurrentActiveCollisions = 0;

    // Clean up invalid weak pointers
    TrackedComponents.RemoveAll([](const TWeakObjectPtr<UPrimitiveComponent>& WeakPtr) {
        return !WeakPtr.IsValid();
    });

    for (const TWeakObjectPtr<UPrimitiveComponent>& WeakComponent : TrackedComponents)
    {
        UPrimitiveComponent* Component = WeakComponent.Get();
        if (Component && Component->GetOwner())
        {
            float DistanceToPlayer = FVector::Dist(Component->GetOwner()->GetActorLocation(), PlayerLocation);
            OptimizeComponentCollision(Component, DistanceToPlayer);
            
            if (Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                CurrentActiveCollisions++;
            }
        }
    }
}

void UPerf_CollisionOptimizer::SetCollisionOptimizationLevel(EPerf_CollisionOptLevel Level)
{
    OptimizationLevel = Level;
    ApplyCollisionProfile(GetCurrentProfile());
    
    UE_LOG(LogTemp, Log, TEXT("CollisionOptimizer: Set optimization level to %d"), (int32)Level);
}

void UPerf_CollisionOptimizer::EnableDynamicLOD(bool bEnable)
{
    bEnableDynamicLOD = bEnable;
    UE_LOG(LogTemp, Log, TEXT("CollisionOptimizer: Dynamic LOD %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerf_CollisionOptimizer::UpdateCollisionComplexity()
{
    FPerf_CollisionProfile CurrentProfile = GetCurrentProfile();
    
    // Adjust complexity based on performance
    if (CurrentPerformanceScore < 0.5f) // Poor performance
    {
        // Reduce collision complexity
        for (const TWeakObjectPtr<UPrimitiveComponent>& WeakComponent : TrackedComponents)
        {
            UPrimitiveComponent* Component = WeakComponent.Get();
            if (Component)
            {
                Component->SetCollisionResponseToAllChannels(ECR_Ignore);
                Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            }
        }
    }
    else if (CurrentPerformanceScore > 0.8f) // Good performance
    {
        // Allow more complex collision
        for (const TWeakObjectPtr<UPrimitiveComponent>& WeakComponent : TrackedComponents)
        {
            UPrimitiveComponent* Component = WeakComponent.Get();
            if (Component)
            {
                Component->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }
    }
}

void UPerf_CollisionOptimizer::CullDistantCollisions()
{
    FPerf_CollisionProfile CurrentProfile = GetCurrentProfile();
    OptimizeCollisionForDistance(CurrentProfile.MaxDistance);
}

int32 UPerf_CollisionOptimizer::GetActiveCollisionCount() const
{
    return CurrentActiveCollisions;
}

float UPerf_CollisionOptimizer::GetCollisionPerformanceScore() const
{
    return CurrentPerformanceScore;
}

void UPerf_CollisionOptimizer::UpdatePerformanceMetrics()
{
    // Calculate performance score based on frame time
    float CurrentFrameTime = FApp::GetDeltaTime();
    float PerformanceRatio = TargetFrameTime / FMath::Max(CurrentFrameTime, 0.001f);
    CurrentPerformanceScore = FMath::Clamp(PerformanceRatio, 0.0f, 2.0f);

    // Log performance metrics
    if (GEngine)
    {
        FString PerformanceText = FString::Printf(TEXT("Collision Performance: %.2f | Active Collisions: %d | Frame Time: %.3fms"), 
            CurrentPerformanceScore, CurrentActiveCollisions, CurrentFrameTime * 1000.0f);
        
        GEngine->AddOnScreenDebugMessage(-1, PerformanceCheckInterval, FColor::Yellow, PerformanceText);
    }

    // Auto-adjust optimization level based on performance
    if (bEnableDynamicLOD)
    {
        if (CurrentPerformanceScore < 0.6f && OptimizationLevel != EPerf_CollisionOptLevel::Low)
        {
            SetCollisionOptimizationLevel(EPerf_CollisionOptLevel::Low);
        }
        else if (CurrentPerformanceScore > 1.2f && OptimizationLevel != EPerf_CollisionOptLevel::High)
        {
            SetCollisionOptimizationLevel(EPerf_CollisionOptLevel::High);
        }
        else if (CurrentPerformanceScore >= 0.8f && CurrentPerformanceScore <= 1.2f && OptimizationLevel != EPerf_CollisionOptLevel::Medium)
        {
            SetCollisionOptimizationLevel(EPerf_CollisionOptLevel::Medium);
        }
    }
}

void UPerf_CollisionOptimizer::ApplyCollisionProfile(const FPerf_CollisionProfile& Profile)
{
    // Apply profile settings to optimization parameters
    TargetFrameTime = Profile.UpdateFrequency;
    
    UE_LOG(LogTemp, Log, TEXT("CollisionOptimizer: Applied profile - MaxDist: %.0f, MaxChecks: %d, Frequency: %.3f"), 
        Profile.MaxDistance, Profile.MaxCollisionChecks, Profile.UpdateFrequency);
}

void UPerf_CollisionOptimizer::OptimizeComponentCollision(UPrimitiveComponent* Component, float Distance)
{
    if (!Component)
    {
        return;
    }

    FPerf_CollisionProfile CurrentProfile = GetCurrentProfile();

    if (Distance > CurrentProfile.MaxDistance)
    {
        // Disable collision for distant objects
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else if (Distance > CurrentProfile.MaxDistance * 0.5f)
    {
        // Simplified collision for medium distance
        Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Component->SetCollisionResponseToAllChannels(ECR_Ignore);
        Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
    else
    {
        // Full collision for close objects
        Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        if (CurrentProfile.bEnableComplexCollision)
        {
            Component->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
}

FPerf_CollisionProfile UPerf_CollisionOptimizer::GetCurrentProfile() const
{
    switch (OptimizationLevel)
    {
        case EPerf_CollisionOptLevel::High:
            return HighQualityProfile;
        case EPerf_CollisionOptLevel::Medium:
            return MediumQualityProfile;
        case EPerf_CollisionOptLevel::Low:
            return LowQualityProfile;
        case EPerf_CollisionOptLevel::Disabled:
        default:
            return LowQualityProfile;
    }
}