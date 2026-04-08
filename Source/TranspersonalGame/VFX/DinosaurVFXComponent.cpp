#include "DinosaurVFXComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "VFXTypes.h"

UDinosaurVFXComponent::UDinosaurVFXComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS para otimização
    
    // Configuração padrão
    DinosaurSize = EDinosaurSize::Medium;
    CurrentState = EDinosaurBehaviorState::Idle;
    HealthPercentage = 1.0f;
    StaminaPercentage = 1.0f;
    IndividualSeed = 0;
    BreathingInterval = 3.0f;
    LastBreathTime = 0.0f;
    MaxEffectDistance = 5000.0f; // 50 metros
    
    // Inicializar componentes
    ActiveBreathingEffect = nullptr;
    ActiveFootstepEffects.Empty();
    
    // Variação procedural padrão
    BreathTint = FLinearColor::White;
    BreathIntensity = 1.0f;
    FootstepScale = 1.0f;
}

void UDinosaurVFXComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Gerar variação procedural baseada no seed
    if (IndividualSeed == 0)
    {
        IndividualSeed = FMath::RandRange(1, 999999);
    }
    GenerateProceduralVariation();
    
    // Iniciar breathing se o dinossauro está vivo
    if (CurrentState != EDinosaurBehaviorState::Dead)
    {
        StartBreathing();
    }
}

void UDinosaurVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Verificar se deve spawnar efeitos baseado na distância
    if (!ShouldSpawnEffect(GetOwner()->GetActorLocation()))
    {
        return;
    }
    
    // Atualizar breathing
    UpdateBreathingEffect();
    
    // Limpar efeitos finalizados
    CleanupFinishedEffects();
}

void UDinosaurVFXComponent::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentState == NewState) return;
    
    EDinosaurBehaviorState PreviousState = CurrentState;
    CurrentState = NewState;
    
    // Ajustar breathing baseado no estado
    switch (NewState)
    {
        case EDinosaurBehaviorState::Running:
        case EDinosaurBehaviorState::Hunting:
        case EDinosaurBehaviorState::Aggressive:
            UpdateBreathingIntensity(2.0f);
            BreathingInterval = 1.5f;
            break;
            
        case EDinosaurBehaviorState::Sleeping:
            UpdateBreathingIntensity(0.3f);
            BreathingInterval = 5.0f;
            break;
            
        case EDinosaurBehaviorState::Dead:
            StopBreathing();
            break;
            
        default:
            UpdateBreathingIntensity(1.0f);
            BreathingInterval = 3.0f;
            break;
    }
    
    // Trigger efeitos especiais para certas transições
    if (PreviousState == EDinosaurBehaviorState::Sleeping && NewState != EDinosaurBehaviorState::Sleeping)
    {
        // Efeito de despertar
        if (VFXSet.SleepingZzz && ActiveBreathingEffect)
        {
            ActiveBreathingEffect->SetFloatParameter(TEXT("WakeUpIntensity"), 2.0f);
        }
    }
}

void UDinosaurVFXComponent::UpdateHealthStatus(float HealthPercent)
{
    HealthPercentage = FMath::Clamp(HealthPercent, 0.0f, 1.0f);
    
    // Ajustar breathing baseado na saúde
    if (HealthPercentage < 0.3f)
    {
        // Respiração pesada quando ferido
        UpdateBreathingIntensity(1.5f);
        BreathingInterval = 2.0f;
        
        if (ActiveBreathingEffect)
        {
            ActiveBreathingEffect->SetColorParameter(TEXT("BreathColor"), 
                FLinearColor::LerpUsingHSV(BreathTint, FLinearColor::Red, 0.4f));
        }
    }
    else
    {
        // Respiração normal
        if (ActiveBreathingEffect)
        {
            ActiveBreathingEffect->SetColorParameter(TEXT("BreathColor"), BreathTint);
        }
    }
}

void UDinosaurVFXComponent::TriggerFootstep(FVector Location, FName SurfaceType)
{
    if (!ShouldSpawnEffect(Location)) return;
    
    UNiagaraSystem* FootstepEffect = GetFootstepEffect(SurfaceType);
    if (!FootstepEffect) return;
    
    // Spawn footstep effect
    UNiagaraComponent* FootstepComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        FootstepEffect,
        Location,
        FRotator::ZeroRotator,
        FVector(FootstepScale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (FootstepComponent)
    {
        // Configurar parâmetros baseados no tamanho do dinossauro
        float SizeMultiplier = 1.0f;
        switch (DinosaurSize)
        {
            case EDinosaurSize::Tiny: SizeMultiplier = 0.3f; break;
            case EDinosaurSize::Small: SizeMultiplier = 0.6f; break;
            case EDinosaurSize::Medium: SizeMultiplier = 1.0f; break;
            case EDinosaurSize::Large: SizeMultiplier = 1.5f; break;
            case EDinosaurSize::Massive: SizeMultiplier = 2.5f; break;
        }
        
        FootstepComponent->SetFloatParameter(TEXT("SizeMultiplier"), SizeMultiplier * FootstepScale);
        FootstepComponent->SetFloatParameter(TEXT("Intensity"), FMath::Lerp(0.5f, 1.5f, StaminaPercentage));
        
        // Adicionar à lista de efeitos ativos
        ActiveFootstepEffects.Add(FootstepComponent);
    }
}

void UDinosaurVFXComponent::TriggerCombatEffect(FName EffectType, FVector Location, FRotator Rotation)
{
    if (!ShouldSpawnEffect(Location)) return;
    
    UNiagaraSystem* CombatEffect = nullptr;
    
    if (EffectType == TEXT("BloodSplatter"))
    {
        CombatEffect = VFXSet.BloodSplatter;
    }
    else if (EffectType == TEXT("ClawScratch"))
    {
        CombatEffect = VFXSet.ClawScratch;
    }
    else if (EffectType == TEXT("BiteImpact"))
    {
        CombatEffect = VFXSet.BiteImpact;
    }
    
    if (CombatEffect)
    {
        UNiagaraComponent* CombatComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            CombatEffect,
            Location,
            Rotation,
            FVector::OneVector,
            true,
            true,
            ENCPoolMethod::None,
            true
        );
        
        if (CombatComponent)
        {
            // Configurar intensidade baseada no tamanho e agressividade
            float Intensity = 1.0f;
            switch (DinosaurSize)
            {
                case EDinosaurSize::Tiny: Intensity = 0.4f; break;
                case EDinosaurSize::Small: Intensity = 0.7f; break;
                case EDinosaurSize::Medium: Intensity = 1.0f; break;
                case EDinosaurSize::Large: Intensity = 1.4f; break;
                case EDinosaurSize::Massive: Intensity = 2.0f; break;
            }
            
            CombatComponent->SetFloatParameter(TEXT("Intensity"), Intensity);
            CombatComponent->SetFloatParameter(TEXT("Aggression"), 
                CurrentState == EDinosaurBehaviorState::Aggressive ? 1.5f : 1.0f);
        }
    }
}

void UDinosaurVFXComponent::SetIndividualVariation(int32 Seed)
{
    IndividualSeed = Seed;
    GenerateProceduralVariation();
}

void UDinosaurVFXComponent::StartBreathing()
{
    if (CurrentState == EDinosaurBehaviorState::Dead) return;
    
    UNiagaraSystem* BreathingSystem = nullptr;
    
    // Escolher sistema de breathing baseado no estado
    switch (CurrentState)
    {
        case EDinosaurBehaviorState::Running:
        case EDinosaurBehaviorState::Hunting:
        case EDinosaurBehaviorState::Aggressive:
            BreathingSystem = VFXSet.HeavyBreathing;
            break;
            
        default:
            BreathingSystem = VFXSet.IdleBreathing;
            break;
    }
    
    if (!BreathingSystem) return;
    
    // Parar breathing anterior
    StopBreathing();
    
    // Encontrar socket de breathing (nariz/boca)
    USkeletalMeshComponent* MeshComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    FVector BreathLocation = GetOwner()->GetActorLocation();
    FRotator BreathRotation = GetOwner()->GetActorRotation();
    
    if (MeshComp && MeshComp->DoesSocketExist(TEXT("Nose")))
    {
        BreathLocation = MeshComp->GetSocketLocation(TEXT("Nose"));
        BreathRotation = MeshComp->GetSocketRotation(TEXT("Nose"));
    }
    else if (MeshComp && MeshComp->DoesSocketExist(TEXT("Head")))
    {
        BreathLocation = MeshComp->GetSocketLocation(TEXT("Head"));
        BreathRotation = MeshComp->GetSocketRotation(TEXT("Head"));
    }
    
    // Spawn breathing effect
    ActiveBreathingEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
        BreathingSystem,
        MeshComp ? MeshComp : GetOwner()->GetRootComponent(),
        MeshComp && MeshComp->DoesSocketExist(TEXT("Nose")) ? TEXT("Nose") : NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true
    );
    
    if (ActiveBreathingEffect)
    {
        // Configurar parâmetros procedurais
        ActiveBreathingEffect->SetColorParameter(TEXT("BreathColor"), BreathTint);
        ActiveBreathingEffect->SetFloatParameter(TEXT("Intensity"), BreathIntensity);
        ActiveBreathingEffect->SetFloatParameter(TEXT("Size"), FootstepScale);
        ActiveBreathingEffect->SetFloatParameter(TEXT("Interval"), BreathingInterval);
    }
}

void UDinosaurVFXComponent::StopBreathing()
{
    if (ActiveBreathingEffect)
    {
        ActiveBreathingEffect->DestroyComponent();
        ActiveBreathingEffect = nullptr;
    }
}

void UDinosaurVFXComponent::UpdateBreathingIntensity(float Intensity)
{
    BreathIntensity = Intensity;
    
    if (ActiveBreathingEffect)
    {
        ActiveBreathingEffect->SetFloatParameter(TEXT("Intensity"), BreathIntensity);
    }
}

void UDinosaurVFXComponent::GenerateProceduralVariation()
{
    // Usar seed para gerar variação consistente
    FRandomStream RandomStream(IndividualSeed);
    
    // Gerar cor de respiração baseada em "genética"
    float Hue = RandomStream.FRandRange(0.0f, 360.0f);
    float Saturation = RandomStream.FRandRange(0.1f, 0.4f);
    float Value = RandomStream.FRandRange(0.8f, 1.0f);
    BreathTint = FLinearColor::MakeFromHSV8(Hue, Saturation * 255, Value * 255);
    
    // Gerar intensidade de respiração
    BreathIntensity = RandomStream.FRandRange(0.7f, 1.3f);
    
    // Gerar escala de pegadas
    FootstepScale = RandomStream.FRandRange(0.8f, 1.2f);
}

void UDinosaurVFXComponent::UpdateBreathingEffect()
{
    if (!ActiveBreathingEffect || CurrentState == EDinosaurBehaviorState::Dead) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Verificar se é hora de uma nova respiração
    if (CurrentTime - LastBreathTime >= BreathingInterval)
    {
        LastBreathTime = CurrentTime;
        
        // Trigger breathing pulse
        if (ActiveBreathingEffect)
        {
            ActiveBreathingEffect->SetFloatParameter(TEXT("TriggerBreath"), 1.0f);
            
            // Reset trigger após um frame
            FTimerHandle ResetHandle;
            GetWorld()->GetTimerManager().SetTimer(ResetHandle, [this]()
            {
                if (ActiveBreathingEffect)
                {
                    ActiveBreathingEffect->SetFloatParameter(TEXT("TriggerBreath"), 0.0f);
                }
            }, 0.1f, false);
        }
    }
}

bool UDinosaurVFXComponent::ShouldSpawnEffect(FVector EffectLocation)
{
    // Verificar distância ao jogador
    float DistanceToPlayer = GetDistanceToPlayer();
    return DistanceToPlayer <= MaxEffectDistance;
}

UNiagaraSystem* UDinosaurVFXComponent::GetFootstepEffect(FName SurfaceType)
{
    if (SurfaceType == TEXT("Water") || SurfaceType == TEXT("Mud"))
    {
        return VFXSet.FootstepSplash;
    }
    else if (SurfaceType == TEXT("Leaves") || SurfaceType == TEXT("Grass"))
    {
        return VFXSet.FootstepLeaves;
    }
    else
    {
        return VFXSet.FootstepDust;
    }
}

void UDinosaurVFXComponent::CleanupFinishedEffects()
{
    // Remover efeitos de footstep que já terminaram
    ActiveFootstepEffects.RemoveAll([](UNiagaraComponent* Component)
    {
        return !Component || !Component->IsActive();
    });
}

float UDinosaurVFXComponent::GetDistanceToPlayer()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return MAX_FLT;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), PC->GetPawn()->GetActorLocation());
}

EVFXPriority UDinosaurVFXComponent::GetEffectPriority(FName EffectType)
{
    if (EffectType == TEXT("BloodSplatter") || EffectType == TEXT("BiteImpact"))
    {
        return EVFXPriority::High;
    }
    else if (EffectType == TEXT("Breathing"))
    {
        return EVFXPriority::Medium;
    }
    else
    {
        return EVFXPriority::Low;
    }
}