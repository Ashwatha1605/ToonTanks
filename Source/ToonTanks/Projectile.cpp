// Copyright Denys Kryvytskyi. All Rights Reserved.

#include "Projectile.h"

#include "Camera/CameraShakeBase.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
    RootComponent = ProjectileMesh;

    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

    TrailParticleSystemComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail Particle System"));
    TrailParticleSystemComp->SetupAttachment(RootComponent);
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();

    ProjectileMesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

    if (LaunchSound) {
        UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
    }
}

float AProjectile::CalculateDamage(float BaseDamage, bool bIsCriticalHit, float CriticalMultiplierValue) const
{
    if (BaseDamage <= 0.0f) {
        return 0.0f;
    }

    const float ValidMultiplier = FMath::Max(1.0f, CriticalMultiplierValue);
    return bIsCriticalHit ? (BaseDamage * ValidMultiplier) : BaseDamage;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (AActor* MyOwner = GetOwner()) {
        AController* OwnerInstigator = MyOwner->GetInstigatorController();
        UClass* DamageTypeClass = UDamageType::StaticClass();

        if (OtherActor && OtherActor != this && OtherActor != MyOwner) {
            const bool bIsCrit = (FMath::FRand() < CriticalChance);
            const float AppliedDamage = CalculateDamage(Damage, bIsCrit, CriticalMultiplier);

            UGameplayStatics::ApplyDamage(OtherActor, AppliedDamage, OwnerInstigator, this, DamageTypeClass);

            if (HitSound) {
                UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
            }

            if (HitParticles) {
                UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, GetActorLocation(), GetActorRotation());
            }

            if (HitCameraShakeClass && GetWorld() && GetWorld()->GetFirstPlayerController()) {
                GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(HitCameraShakeClass);
            }
        }
    }

    Destroy();
}
