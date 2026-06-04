#include "NPC_DinosaurSensorySystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurSensorySystem::UNPC_DinosaurSensorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default sensory ranges
    SightRange = 3000.0f;
    HearingRange = 2000.0f;
    SmellRange = 1500.0f;
    SightAngle = 120.0f;
    
    // Default sensory acuity
    SightAcuity = 0.8f;
    HearingAcuity = 0.7f;
    SmellAcuity = 0.6f;
    
    bDebugSensors = false;
}

void UNPC_DinosaurSensorySystem::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurSensorySystem: Owner is not a Pawn"));
    }
    
    // Initialize sensory data
    DetectedTargets.Empty();
    SensoryMemory.Empty();
}

void UNPC_DinosaurSensorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn)
        return;
        
    // Update all sensory systems
    UpdateSightSense();
    UpdateHearingSense();
    UpdateSmellSense();
    
    // Update sensory memory
    UpdateSensoryMemory(DeltaTime);
    
    // Debug visualization
    if (bDebugSensors)
    {
        DrawDebugSensors();
    }
}

void UNPC_DinosaurSensorySystem::UpdateSightSense()
{
    if (!OwnerPawn)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    FVector OwnerForward = OwnerPawn->GetActorForwardVector();
    
    // Get all pawns in range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == OwnerPawn)
            continue;
            
        APawn* TargetPawn = Cast<APawn>(Actor);
        if (!TargetPawn)
            continue;
            
        FVector TargetLocation = TargetPawn->GetActorLocation();
        float Distance = FVector::Dist(OwnerLocation, TargetLocation);
        
        // Check if within sight range
        if (Distance > SightRange)
            continue;
            
        // Check if within sight angle
        FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
        float DotProduct = FVector::DotProduct(OwnerForward, DirectionToTarget);
        float AngleToTarget = FMath::Acos(DotProduct) * 180.0f / PI;
        
        if (AngleToTarget > SightAngle * 0.5f)
            continue;
            
        // Line of sight check
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwnerPawn);
        
        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            OwnerLocation + FVector(0, 0, 50), // Slightly elevated
            TargetLocation + FVector(0, 0, 50),
            ECollisionChannel::ECC_Visibility,
            QueryParams
        );
        
        // If we hit something other than the target, sight is blocked
        if (bHit && HitResult.GetActor() != TargetPawn)
            continue;
            
        // Target is visible
        ProcessSensoryDetection(TargetPawn, ENPC_SenseType::Sight, Distance);
    }
}

void UNPC_DinosaurSensorySystem::UpdateHearingSense()
{
    if (!OwnerPawn)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    
    // Get all pawns in hearing range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == OwnerPawn)
            continue;
            
        APawn* TargetPawn = Cast<APawn>(Actor);
        if (!TargetPawn)
            continue;
            
        FVector TargetLocation = TargetPawn->GetActorLocation();
        float Distance = FVector::Dist(OwnerLocation, TargetLocation);
        
        // Check if within hearing range
        if (Distance > HearingRange)
            continue;
            
        // Calculate noise level based on target movement
        FVector TargetVelocity = TargetPawn->GetVelocity();
        float NoiseLevel = TargetVelocity.Size() / 600.0f; // Normalize to 0-1 range
        
        // Adjust detection based on noise and distance
        float HearingChance = (NoiseLevel * HearingAcuity) * (1.0f - Distance / HearingRange);
        
        if (FMath::RandRange(0.0f, 1.0f) < HearingChance)
        {
            ProcessSensoryDetection(TargetPawn, ENPC_SenseType::Hearing, Distance);
        }
    }
}

void UNPC_DinosaurSensorySystem::UpdateSmellSense()
{
    if (!OwnerPawn)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    
    // Get all pawns in smell range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == OwnerPawn)
            continue;
            
        APawn* TargetPawn = Cast<APawn>(Actor);
        if (!TargetPawn)
            continue;
            
        FVector TargetLocation = TargetPawn->GetActorLocation();
        float Distance = FVector::Dist(OwnerLocation, TargetLocation);
        
        // Check if within smell range
        if (Distance > SmellRange)
            continue;
            
        // Smell detection is less precise but works through obstacles
        float SmellChance = SmellAcuity * (1.0f - Distance / SmellRange);
        
        if (FMath::RandRange(0.0f, 1.0f) < SmellChance)
        {
            ProcessSensoryDetection(TargetPawn, ENPC_SenseType::Smell, Distance);
        }
    }
}

void UNPC_DinosaurSensorySystem::ProcessSensoryDetection(APawn* DetectedPawn, ENPC_SenseType SenseType, float Distance)
{
    if (!DetectedPawn)
        return;
        
    // Check if target is already detected
    FNPC_SensoryTarget* ExistingTarget = DetectedTargets.FindByPredicate([DetectedPawn](const FNPC_SensoryTarget& Target)
    {
        return Target.TargetPawn == DetectedPawn;
    });
    
    if (ExistingTarget)
    {
        // Update existing detection
        ExistingTarget->LastDetectedTime = GetWorld()->GetTimeSeconds();
        ExistingTarget->Distance = Distance;
        ExistingTarget->DetectionStrength = FMath::Max(ExistingTarget->DetectionStrength, GetDetectionStrength(SenseType, Distance));
        
        // Add sense type if not already present
        ExistingTarget->DetectedSenses.AddUnique(SenseType);
    }
    else
    {
        // Add new detection
        FNPC_SensoryTarget NewTarget;
        NewTarget.TargetPawn = DetectedPawn;
        NewTarget.LastDetectedTime = GetWorld()->GetTimeSeconds();
        NewTarget.Distance = Distance;
        NewTarget.DetectionStrength = GetDetectionStrength(SenseType, Distance);
        NewTarget.DetectedSenses.Add(SenseType);
        
        DetectedTargets.Add(NewTarget);
        
        // Broadcast detection event
        OnTargetDetected.Broadcast(DetectedPawn, SenseType, Distance);
    }
    
    // Add to sensory memory
    AddToSensoryMemory(DetectedPawn, SenseType);
}

float UNPC_DinosaurSensorySystem::GetDetectionStrength(ENPC_SenseType SenseType, float Distance) const
{
    float MaxRange = 0.0f;
    float Acuity = 0.0f;
    
    switch (SenseType)
    {
        case ENPC_SenseType::Sight:
            MaxRange = SightRange;
            Acuity = SightAcuity;
            break;
        case ENPC_SenseType::Hearing:
            MaxRange = HearingRange;
            Acuity = HearingAcuity;
            break;
        case ENPC_SenseType::Smell:
            MaxRange = SmellRange;
            Acuity = SmellAcuity;
            break;
    }
    
    return Acuity * (1.0f - Distance / MaxRange);
}

void UNPC_DinosaurSensorySystem::UpdateSensoryMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old detections
    DetectedTargets.RemoveAll([CurrentTime](const FNPC_SensoryTarget& Target)
    {
        return (CurrentTime - Target.LastDetectedTime) > 5.0f; // 5 second timeout
    });
    
    // Decay sensory memory
    for (auto& MemoryEntry : SensoryMemory)
    {
        MemoryEntry.Value.Confidence -= DeltaTime * 0.1f; // Decay rate
        MemoryEntry.Value.Confidence = FMath::Max(0.0f, MemoryEntry.Value.Confidence);
    }
    
    // Remove completely decayed memories
    SensoryMemory.RemoveAll([](const TPair<TWeakObjectPtr<APawn>, FNPC_SensoryMemory>& Entry)
    {
        return Entry.Value.Confidence <= 0.0f || !Entry.Key.IsValid();
    });
}

void UNPC_DinosaurSensorySystem::AddToSensoryMemory(APawn* TargetPawn, ENPC_SenseType SenseType)
{
    if (!TargetPawn)
        return;
        
    TWeakObjectPtr<APawn> TargetPtr(TargetPawn);
    
    FNPC_SensoryMemory* Memory = SensoryMemory.Find(TargetPtr);
    if (Memory)
    {
        // Update existing memory
        Memory->LastSeenLocation = TargetPawn->GetActorLocation();
        Memory->LastSeenTime = GetWorld()->GetTimeSeconds();
        Memory->Confidence = FMath::Min(1.0f, Memory->Confidence + 0.2f);
        Memory->DetectedSenses.AddUnique(SenseType);
    }
    else
    {
        // Create new memory
        FNPC_SensoryMemory NewMemory;
        NewMemory.LastSeenLocation = TargetPawn->GetActorLocation();
        NewMemory.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewMemory.Confidence = 0.5f;
        NewMemory.DetectedSenses.Add(SenseType);
        
        SensoryMemory.Add(TargetPtr, NewMemory);
    }
}

void UNPC_DinosaurSensorySystem::DrawDebugSensors()
{
    if (!OwnerPawn)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    FVector OwnerForward = OwnerPawn->GetActorForwardVector();
    
    // Draw sight cone
    DrawDebugCone(World, OwnerLocation, OwnerForward, SightRange, 
                  FMath::DegreesToRadians(SightAngle * 0.5f), 
                  FMath::DegreesToRadians(SightAngle * 0.5f), 
                  16, FColor::Green, false, -1.0f, 0, 2.0f);
    
    // Draw hearing range
    DrawDebugSphere(World, OwnerLocation, HearingRange, 32, FColor::Yellow, false, -1.0f, 0, 1.0f);
    
    // Draw smell range
    DrawDebugSphere(World, OwnerLocation, SmellRange, 32, FColor::Blue, false, -1.0f, 0, 1.0f);
    
    // Draw detected targets
    for (const FNPC_SensoryTarget& Target : DetectedTargets)
    {
        if (Target.TargetPawn.IsValid())
        {
            FVector TargetLocation = Target.TargetPawn->GetActorLocation();
            DrawDebugLine(World, OwnerLocation, TargetLocation, FColor::Red, false, -1.0f, 0, 3.0f);
            DrawDebugSphere(World, TargetLocation, 50.0f, 12, FColor::Red, false, -1.0f, 0, 2.0f);
        }
    }
}

TArray<FNPC_SensoryTarget> UNPC_DinosaurSensorySystem::GetDetectedTargets() const
{
    return DetectedTargets;
}

APawn* UNPC_DinosaurSensorySystem::GetClosestDetectedTarget() const
{
    if (DetectedTargets.Num() == 0)
        return nullptr;
        
    const FNPC_SensoryTarget* ClosestTarget = &DetectedTargets[0];
    for (const FNPC_SensoryTarget& Target : DetectedTargets)
    {
        if (Target.Distance < ClosestTarget->Distance)
        {
            ClosestTarget = &Target;
        }
    }
    
    return ClosestTarget->TargetPawn.Get();
}

bool UNPC_DinosaurSensorySystem::IsTargetDetected(APawn* TargetPawn) const
{
    return DetectedTargets.ContainsByPredicate([TargetPawn](const FNPC_SensoryTarget& Target)
    {
        return Target.TargetPawn == TargetPawn;
    });
}

void UNPC_DinosaurSensorySystem::SetSensoryRanges(float NewSightRange, float NewHearingRange, float NewSmellRange)
{
    SightRange = NewSightRange;
    HearingRange = NewHearingRange;
    SmellRange = NewSmellRange;
}

void UNPC_DinosaurSensorySystem::SetSensoryAcuity(float NewSightAcuity, float NewHearingAcuity, float NewSmellAcuity)
{
    SightAcuity = FMath::Clamp(NewSightAcuity, 0.0f, 1.0f);
    HearingAcuity = FMath::Clamp(NewHearingAcuity, 0.0f, 1.0f);
    SmellAcuity = FMath::Clamp(NewSmellAcuity, 0.0f, 1.0f);
}