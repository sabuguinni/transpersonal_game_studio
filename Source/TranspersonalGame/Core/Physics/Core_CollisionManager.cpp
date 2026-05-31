#include "Core_CollisionManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UCore_CollisionManager::UCore_CollisionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    MaxCollisionHistorySize = 1000;
    bDebugVisualization = false;
    CollisionHistoryRetentionTime = 30.0f;
}

void UCore_CollisionManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultProfiles();
    
    // Bind collision events for owner actor's components
    if (AActor* Owner = GetOwner())
    {
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                PrimComp->OnComponentHit.AddDynamic(this, &UCore_CollisionManager::OnComponentHit);
                PrimComp->OnComponentBeginOverlap.AddDynamic(this, &UCore_CollisionManager::OnComponentBeginOverlap);
                PrimComp->OnComponentEndOverlap.AddDynamic(this, &UCore_CollisionManager::OnComponentEndOverlap);
            }
        }
    }
}

void UCore_CollisionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupOldCollisions();
    
    if (bDebugVisualization)
    {
        DebugDrawCollisionShapes();
    }
}

void UCore_CollisionManager::SetCollisionProfile(UPrimitiveComponent* Component, const FCore_CollisionProfile& Profile)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: Null component passed to SetCollisionProfile"));
        return;
    }
    
    Component->SetCollisionEnabled(Profile.CollisionEnabled);
    Component->SetCollisionObjectType(Profile.ObjectType);
    Component->SetGenerateOverlapEvents(Profile.bGenerateOverlapEvents);
    Component->SetCanCharacterStepUpOn(Profile.bCanCharacterStepUpOn ? ECB_Yes : ECB_No);
}

FCore_CollisionProfile UCore_CollisionManager::GetCollisionProfile(UPrimitiveComponent* Component)
{
    FCore_CollisionProfile Profile;
    
    if (Component)
    {
        Profile.CollisionEnabled = Component->GetCollisionEnabled();
        Profile.ObjectType = Component->GetCollisionObjectType();
        Profile.bGenerateOverlapEvents = Component->GetGenerateOverlapEvents();
        Profile.bCanCharacterStepUpOn = (Component->GetCanCharacterStepUpOn() == ECB_Yes);
    }
    
    return Profile;
}

void UCore_CollisionManager::RegisterCollisionProfile(const FName& ProfileName, const FCore_CollisionProfile& Profile)
{
    RegisteredProfiles.Add(ProfileName, Profile);
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Registered collision profile '%s'"), *ProfileName.ToString());
}

bool UCore_CollisionManager::LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult, bool bTraceComplex)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = bTraceComplex;
        QueryParams.AddIgnoredActor(GetOwner());
        
        return World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    }
    
    return false;
}

bool UCore_CollisionManager::SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        return World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), QueryParams);
    }
    
    return false;
}

bool UCore_CollisionManager::BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, const FRotator& Orientation, FHitResult& HitResult)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        return World->SweepSingleByChannel(HitResult, Start, End, Orientation.Quaternion(), ECC_Visibility, FCollisionShape::MakeBox(HalfSize), QueryParams);
    }
    
    return false;
}

bool UCore_CollisionManager::SphereOverlap(const FVector& Location, float Radius, TArray<AActor*>& OverlappingActors)
{
    if (UWorld* World = GetWorld())
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        bool bHasOverlap = World->OverlapMultiByChannel(OverlapResults, Location, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), QueryParams);
        
        OverlappingActors.Empty();
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                OverlappingActors.AddUnique(Result.GetActor());
            }
        }
        
        return bHasOverlap;
    }
    
    return false;
}

bool UCore_CollisionManager::BoxOverlap(const FVector& Location, const FVector& HalfSize, const FRotator& Orientation, TArray<AActor*>& OverlappingActors)
{
    if (UWorld* World = GetWorld())
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        bool bHasOverlap = World->OverlapMultiByChannel(OverlapResults, Location, Orientation.Quaternion(), ECC_Visibility, FCollisionShape::MakeBox(HalfSize), QueryParams);
        
        OverlappingActors.Empty();
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                OverlappingActors.AddUnique(Result.GetActor());
            }
        }
        
        return bHasOverlap;
    }
    
    return false;
}

void UCore_CollisionManager::SetCollisionResponseToChannel(UPrimitiveComponent* Component, ECollisionChannel Channel, ECollisionResponse Response)
{
    if (Component)
    {
        Component->SetCollisionResponseToChannel(Channel, Response);
    }
}

void UCore_CollisionManager::SetCollisionResponseToAllChannels(UPrimitiveComponent* Component, ECollisionResponse Response)
{
    if (Component)
    {
        Component->SetCollisionResponseToAllChannels(Response);
    }
}

void UCore_CollisionManager::SetPhysicsMaterial(UPrimitiveComponent* Component, UPhysicalMaterial* PhysMaterial)
{
    if (Component && PhysMaterial)
    {
        Component->SetPhysMaterialOverride(PhysMaterial);
    }
}

void UCore_CollisionManager::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    FCore_CollisionData CollisionData;
    CollisionData.HitActor = OtherActor;
    CollisionData.HitComponent = OtherComp;
    CollisionData.ImpactPoint = Hit.ImpactPoint;
    CollisionData.ImpactNormal = Hit.ImpactNormal;
    CollisionData.ImpactForce = NormalImpulse.Size();
    CollisionData.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    RecordCollision(CollisionData);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Hit detected with %s at force %.2f"), 
           OtherActor ? *OtherActor->GetName() : TEXT("Unknown"), CollisionData.ImpactForce);
}

void UCore_CollisionManager::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Begin overlap with %s"), 
           OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
}

void UCore_CollisionManager::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: End overlap with %s"), 
           OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
}

void UCore_CollisionManager::RecordCollision(const FCore_CollisionData& CollisionData)
{
    CollisionHistory.Add(CollisionData);
    
    if (CollisionHistory.Num() > MaxCollisionHistorySize)
    {
        CollisionHistory.RemoveAt(0, CollisionHistory.Num() - MaxCollisionHistorySize);
    }
}

TArray<FCore_CollisionData> UCore_CollisionManager::GetRecentCollisions(float TimeWindow)
{
    TArray<FCore_CollisionData> RecentCollisions;
    
    if (UWorld* World = GetWorld())
    {
        float CurrentTime = World->GetTimeSeconds();
        float CutoffTime = CurrentTime - TimeWindow;
        
        for (const FCore_CollisionData& Collision : CollisionHistory)
        {
            if (Collision.Timestamp >= CutoffTime)
            {
                RecentCollisions.Add(Collision);
            }
        }
    }
    
    return RecentCollisions;
}

void UCore_CollisionManager::ClearCollisionHistory()
{
    CollisionHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Collision history cleared"));
}

void UCore_CollisionManager::DebugDrawCollisionShapes()
{
    if (UWorld* World = GetWorld())
    {
        if (AActor* Owner = GetOwner())
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                DrawDebugCollisionShape(PrimComp);
            }
        }
    }
}

void UCore_CollisionManager::SetDebugVisualization(bool bEnabled)
{
    bDebugVisualization = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Debug visualization %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_CollisionManager::InitializeDefaultProfiles()
{
    // Character Profile
    FCore_CollisionProfile CharacterProfile;
    CharacterProfile.ProfileName = TEXT("Character");
    CharacterProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    CharacterProfile.ObjectType = ECR_Pawn;
    CharacterProfile.bGenerateOverlapEvents = true;
    CharacterProfile.bCanCharacterStepUpOn = false;
    RegisterCollisionProfile(CharacterProfile.ProfileName, CharacterProfile);
    
    // Environment Profile
    FCore_CollisionProfile EnvironmentProfile;
    EnvironmentProfile.ProfileName = TEXT("Environment");
    EnvironmentProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    EnvironmentProfile.ObjectType = ECR_WorldStatic;
    EnvironmentProfile.bGenerateOverlapEvents = false;
    EnvironmentProfile.bCanCharacterStepUpOn = true;
    RegisterCollisionProfile(EnvironmentProfile.ProfileName, EnvironmentProfile);
    
    // Projectile Profile
    FCore_CollisionProfile ProjectileProfile;
    ProjectileProfile.ProfileName = TEXT("Projectile");
    ProjectileProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    ProjectileProfile.ObjectType = ECR_WorldDynamic;
    ProjectileProfile.bGenerateOverlapEvents = true;
    ProjectileProfile.bCanCharacterStepUpOn = false;
    RegisterCollisionProfile(ProjectileProfile.ProfileName, ProjectileProfile);
}

void UCore_CollisionManager::CleanupOldCollisions()
{
    if (UWorld* World = GetWorld())
    {
        float CurrentTime = World->GetTimeSeconds();
        float CutoffTime = CurrentTime - CollisionHistoryRetentionTime;
        
        CollisionHistory.RemoveAll([CutoffTime](const FCore_CollisionData& Collision)
        {
            return Collision.Timestamp < CutoffTime;
        });
    }
}

void UCore_CollisionManager::DrawDebugCollisionShape(UPrimitiveComponent* Component)
{
    if (!Component || !GetWorld())
    {
        return;
    }
    
    FVector Location = Component->GetComponentLocation();
    FRotator Rotation = Component->GetComponentRotation();
    
    // Draw different shapes based on component type
    if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Component))
    {
        FBoxSphereBounds Bounds = StaticMeshComp->GetLocalBounds();
        DrawDebugBox(GetWorld(), Location, Bounds.BoxExtent, Rotation.Quaternion(), FColor::Green, false, 0.1f);
    }
    else if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(Component))
    {
        FBoxSphereBounds Bounds = SkeletalMeshComp->GetLocalBounds();
        DrawDebugBox(GetWorld(), Location, Bounds.BoxExtent, Rotation.Quaternion(), FColor::Blue, false, 0.1f);
    }
    else
    {
        // Generic sphere for other components
        DrawDebugSphere(GetWorld(), Location, 50.0f, 12, FColor::Red, false, 0.1f);
    }
}