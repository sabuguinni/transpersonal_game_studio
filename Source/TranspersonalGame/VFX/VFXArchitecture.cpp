#include "VFXArchitecture.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UVFXManagerComponent::UVFXManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    CurrentLODLevel = EVFXLODLevel::High;
    InitializeLODSettings();
}

void UVFXManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize VFX systems paths
    VFXSystems.Add(EVFXCategory::DinosaurBreathing, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Dinosaurs/NS_DinosaurBreath"))));
    VFXSystems.Add(EVFXCategory::DinosaurFootsteps, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Dinosaurs/NS_DinosaurFootsteps"))));
    VFXSystems.Add(EVFXCategory::AmbientDust, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Environment/NS_AmbientDust"))));
    VFXSystems.Add(EVFXCategory::TensionParticles, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Emotional/NS_TensionAtmosphere"))));
}

void UVFXManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Calculate distance to player for LOD management
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
                UpdateVFXBasedOnDistance(DistanceToPlayer);
            }
        }
    }
    
    CleanupInactiveVFX();
}

void UVFXManagerComponent::SpawnVFX(EVFXCategory Category, FVector Location, FRotator Rotation)
{
    if (TSoftObjectPtr<UNiagaraSystem>* SystemPtr = VFXSystems.Find(Category))
    {
        if (UNiagaraSystem* System = SystemPtr->LoadSynchronous())
        {
            UNiagaraComponent* NewVFXComponent = CreateVFXComponent(System, Location, Rotation);
            if (NewVFXComponent)
            {
                ActiveVFXComponents.Add(NewVFXComponent);
                
                // Apply current LOD settings
                const FVFXLODSettings& CurrentSettings = LODSettings[CurrentLODLevel];
                
                // Set particle count limit
                NewVFXComponent->SetIntParameter(TEXT("MaxParticles"), CurrentSettings.MaxParticles);
                
                // Set update frequency
                NewVFXComponent->SetFloatParameter(TEXT("UpdateFrequency"), CurrentSettings.UpdateFrequency);
                
                // Enable/disable collision
                NewVFXComponent->SetBoolParameter(TEXT("EnableCollision"), CurrentSettings.bEnableCollision);
                
                // Shadow casting
                NewVFXComponent->SetCastShadow(CurrentSettings.bCastShadows);
            }
        }
    }
}

void UVFXManagerComponent::SetVFXLOD(EVFXLODLevel NewLODLevel)
{
    if (CurrentLODLevel == NewLODLevel) return;
    
    CurrentLODLevel = NewLODLevel;
    
    // Update all active VFX components
    const FVFXLODSettings& NewSettings = LODSettings[CurrentLODLevel];
    
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->SetIntParameter(TEXT("MaxParticles"), NewSettings.MaxParticles);
            VFXComp->SetFloatParameter(TEXT("UpdateFrequency"), NewSettings.UpdateFrequency);
            VFXComp->SetBoolParameter(TEXT("EnableCollision"), NewSettings.bEnableCollision);
            VFXComp->SetCastShadow(NewSettings.bCastShadows);
            
            if (CurrentLODLevel == EVFXLODLevel::Disabled)
            {
                VFXComp->SetVisibility(false);
            }
            else
            {
                VFXComp->SetVisibility(true);
            }
        }
    }
}

void UVFXManagerComponent::UpdateVFXBasedOnDistance(float DistanceToPlayer)
{
    EVFXLODLevel NewLODLevel;
    
    if (DistanceToPlayer <= 5000.0f) // 50m
    {
        NewLODLevel = EVFXLODLevel::High;
    }
    else if (DistanceToPlayer <= 20000.0f) // 200m
    {
        NewLODLevel = EVFXLODLevel::Medium;
    }
    else
    {
        NewLODLevel = EVFXLODLevel::Low;
    }
    
    SetVFXLOD(NewLODLevel);
}

int32 UVFXManagerComponent::GetActiveParticleCount() const
{
    int32 TotalParticles = 0;
    
    for (const UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp) && VFXComp->IsActive())
        {
            // This is an approximation - Niagara doesn't expose exact particle count easily
            TotalParticles += LODSettings[CurrentLODLevel].MaxParticles;
        }
    }
    
    return TotalParticles;
}

float UVFXManagerComponent::GetVFXPerformanceMetric() const
{
    // Simple performance metric: active components * particles per component
    int32 ActiveComponents = 0;
    for (const UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp) && VFXComp->IsActive())
        {
            ActiveComponents++;
        }
    }
    
    return ActiveComponents * LODSettings[CurrentLODLevel].MaxParticles;
}

void UVFXManagerComponent::InitializeLODSettings()
{
    // High LOD (0-50m) - Full detail
    FVFXLODSettings HighLOD;
    HighLOD.MaxParticles = 1000;
    HighLOD.UpdateFrequency = 60.0f;
    HighLOD.bEnableCollision = true;
    HighLOD.bCastShadows = true;
    LODSettings.Add(EVFXLODLevel::High, HighLOD);
    
    // Medium LOD (50-200m) - Reduced detail
    FVFXLODSettings MediumLOD;
    MediumLOD.MaxParticles = 500;
    MediumLOD.UpdateFrequency = 30.0f;
    MediumLOD.bEnableCollision = false;
    MediumLOD.bCastShadows = false;
    LODSettings.Add(EVFXLODLevel::Medium, MediumLOD);
    
    // Low LOD (200m+) - Minimal detail
    FVFXLODSettings LowLOD;
    LowLOD.MaxParticles = 100;
    LowLOD.UpdateFrequency = 15.0f;
    LowLOD.bEnableCollision = false;
    LowLOD.bCastShadows = false;
    LODSettings.Add(EVFXLODLevel::Low, LowLOD);
    
    // Disabled LOD
    FVFXLODSettings DisabledLOD;
    DisabledLOD.MaxParticles = 0;
    DisabledLOD.UpdateFrequency = 0.0f;
    DisabledLOD.bEnableCollision = false;
    DisabledLOD.bCastShadows = false;
    LODSettings.Add(EVFXLODLevel::Disabled, DisabledLOD);
}

void UVFXManagerComponent::CleanupInactiveVFX()
{
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* VFXComp)
    {
        return !IsValid(VFXComp) || !VFXComp->IsActive();
    });
}

UNiagaraComponent* UVFXManagerComponent::CreateVFXComponent(UNiagaraSystem* System, FVector Location, FRotator Rotation)
{
    if (!System || !GetOwner()) return nullptr;
    
    UNiagaraComponent* NewComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    return NewComponent;
}